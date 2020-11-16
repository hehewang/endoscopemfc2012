/*
 * COM串口操作及相关的协议分析
 * 文件类型: .cpp文件
 */

#include "pch.h"
#include "LightComIO.h"
#include <algorithm>
#include <afxtempl.h>
#include <vector>

using namespace std;

//CLightComIO g_com;
//CWinThread* g_m_comThread;
//HANDLE g_hCom;   // 串口句柄
/*std::vector<CRetryInfo> g_RetryList;*/


CLightComIO::CLightComIO()
{
	// 初始化
	m_hCom = INVALID_HANDLE_VALUE;
	m_nBaudRate = CBR_9600; // 初始化波特率
	m_bExitFlag = TRUE;
	m_comThread = NULL;
	m_pDataHandler = NULL;
}

CLightComIO::~CLightComIO()
{
	Release();
}

bool CLightComIO::Initialize(CString comName, int nBaudRate)
{

	// 初始化变量
	m_rSize = 0;
	memset(m_rBuf, 0, sizeof(m_rBuf));

	// 打开串口
	m_hCom = CreateFile(comName,
						GENERIC_READ | GENERIC_WRITE,
						0,    // Exclusive access 
						NULL, // No security attributes 
						OPEN_EXISTING,
						0,    // FILE_FLAG_OVERLAPPED
						NULL);
	if (m_hCom == INVALID_HANDLE_VALUE) {
		// 打开失败
//		AfxMessageBox(_T("打开COM失败"));
		return false;
	}

	if(nBaudRate>0) {
		// 设置比特率
		m_nBaudRate = nBaudRate;
	}

	// 取得指定串口的当前状态
	DCB dcb;
	if(!GetCommState(m_hCom,(LPDCB)&dcb)) {
		return false;
	}

	dcb.BaudRate = (DWORD)m_nBaudRate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fBinary = 1;

	// 设置指定串口状态
	if(!SetCommState(m_hCom, &dcb)) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					  FORMAT_MESSAGE_FROM_SYSTEM | 
					  FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL,
					  GetLastError(),
					  0, 
					  (LPTSTR)&lpMsgBuf,
					  0,
					  NULL);
		MessageBox(NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION);
		LocalFree(lpMsgBuf);
		return false;
	}
	
	// 设置超时
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout			= MAXDWORD; // 读间隔超时
	timeouts.ReadTotalTimeoutMultiplier		= 0; // 读时间系数
	timeouts.ReadTotalTimeoutConstant		= 0; // 读时间常量
	timeouts.WriteTotalTimeoutMultiplier	= 0; // 写时间系数
	timeouts.WriteTotalTimeoutConstant		= 1000; // 写时间常量(1秒后写超时)

	if (!SetCommTimeouts(m_hCom, &timeouts)) {
		// 设置超时失败
		return false;
	}

	// 设置串口读缓冲和写缓冲的大小
	SetupComm(m_hCom, 1024*4, 512*4);

	// 清空串口读写缓冲区
	PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);

	// 启动线程
	/*tmpTheApp->*/m_comThread = AfxBeginThread(CLightComIO::RecvHandler, this, THREAD_PRIORITY_HIGHEST);
	m_bExitFlag = FALSE;

	return true;
}

// 释放串口
void CLightComIO::Release()
{	
	if(!m_bExitFlag)
	{
		m_bExitFlag = TRUE;
		::WaitForSingleObject(m_comThread->m_hThread,10);
	}
	if(m_hCom != INVALID_HANDLE_VALUE) 
	{
		CloseHandle(m_hCom);
		m_hCom = INVALID_HANDLE_VALUE;
	}

}

// 数据发送
BOOL CLightComIO::SendData(BYTE* buf, int size)
{	
	DWORD dwWrite = 0;
	//添加发送协议
	BYTE* pBuf;
	
	//长度(不包含两位帧头)
	WORD sndsize = size;
	
	BYTE sndBuf[32];
	memset(sndBuf, 0, sizeof(sndBuf));
	pBuf = sndBuf;
	
	//帧起始
	*(BYTE*)pBuf = 0xA5;
	pBuf++;
	*(BYTE*)pBuf = 0x5A;
	pBuf++;

	for( int i=0; i<sndsize; i++)
	{
		*(BYTE*)pBuf = buf[i];
		pBuf++;		
	}
	//帧结束
// 	*(BYTE*)pBuf = 0x97;
// 	pBuf++;
// 	*(BYTE*)pBuf = 0x98;
// 	pBuf++;
	BOOL RTN = WriteFile(m_hCom, (LPCVOID)sndBuf, sndsize+2, &dwWrite, NULL);

	//打出log代码，实际发布时屏蔽
	/*
	CString cstemp;
	CString csbuf;
	for(int j = 0; j<(pBuf-sndBuf); j++)
	{
		cstemp.Format(_T("%02X "),(unsigned char)sndBuf[j]);
		csbuf = csbuf + cstemp;
	}
	
	TRACE(csbuf);	
	CString filePath;
	filePath = ReturnPath();
	filePath += "\\Voice\\SendCOMLOG.txt";
	WriteComLog(csbuf,filePath);*/
	
	return RTN;
}

// 串口接收监控线程
UINT CLightComIO::RecvHandler(LPVOID lpParam)
{
	return ((CLightComIO*)(lpParam))->OnRecv(lpParam);
}

// 当接收到串口数据时
UINT CLightComIO::OnRecv(LPVOID lpParam)
{
	CLightComIO* pCom =(CLightComIO*)lpParam;
	DWORD dwSize=0;
	BOOL bRet = TRUE;
	BYTE buf[32] = {0};
	CString dbgbuf,dbgtemp;

	while (1) 
	{		
		dwSize = 0;
 		bRet = ReadFile(m_hCom,			  // 串口句柄
						m_rBuf + m_rSize, // 缓冲
						1,				  // 完成一次读串口操作需要读出的字节数总数
						&dwSize,	      // 完成一次读串口操作后实际读出的字节总数
						NULL );


		// 串口无效，关闭串口就退出了线程
		if(!bRet) {
			return 0;
		}

		// 如果读到数据
		if(dwSize > 0) 
		{
			m_rSize += dwSize;
		
			//检查起始位A55A
			if( m_rSize==1 )
			{
				if(m_rBuf[0] != 0xA5 )
				{
					//把接收数位置零
					m_rSize = 0;
					//清BUF
					memset(m_rBuf,0,sizeof(m_rBuf));
				}
			}
			
			if(m_rSize==2)
			{
				if(m_rBuf[1] != 0x5A )
				{
					m_rSize = 0;
					memset(m_rBuf,0,sizeof(m_rBuf));
				}
			}

			//接收指定长度帧
			if( m_rBuf[2]==(m_rSize-3) )
			{					
				for( int j = 0; j < m_rSize ; j++ )
				{
					buf[j] = m_rBuf[j];
				}

				//给主窗口发送消息
				if ( m_pDataHandler )
					m_pDataHandler->ProcessData(buf,m_rSize);

				m_rSize = 0;
				memset(m_rBuf,0,sizeof(m_rBuf));


				//打出log代码，实际发布时屏蔽
				/*dbgbuf.Empty();
				dbgtemp.Empty();
				for(int x = 0; x < j+2; x++)
				{
					dbgtemp.Format(_T("%02X "),buf[x]);
					dbgbuf = dbgbuf + dbgtemp;
				}
				CString filePath;
				filePath = ReturnPath();
				filePath += "\\Voice\\COMLOG.txt";
				WriteComLog(dbgbuf,filePath);
				TRACE(dbgbuf);*/
				//缓冲清理					
				memset(buf,0,32);	
			}


			//接收长度太长的情况下清空
			if( m_rSize>=64 )
			{
				m_rSize=0;
				memset(m_rBuf, 0, sizeof(m_rBuf));
			}
		}
		// 没有读到数据
		else {
			Sleep(1);
		}
	}
	return 1;
}
