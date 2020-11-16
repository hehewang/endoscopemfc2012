
/*
 * COM串口操作及相关的协议分析
 * 文件类型: .cpp文件
 */
#include "pch.h"
#include "ProcessCard.h"
#include <algorithm>
#include <afxtempl.h>
#include <vector>
#include <sys/timeb.h>
#include <fstream>
#include<iostream>
using namespace std;

CProcessCard::CProcessCard(void)
{
	// 初始化
	m_hCom = INVALID_HANDLE_VALUE;
	m_nBaudRate = CBR_9600; // 初始化波特率
	m_bExitFlag = TRUE;
	m_comThread = NULL;
	m_pDataHandler = NULL;
}


CProcessCard::~CProcessCard(void)
{
	Release();
}




bool CProcessCard::Initialize(CString comName, int nBaudRate)
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
	/*tmpTheApp->*/m_comThread = AfxBeginThread(CProcessCard::RecvHandler, this, THREAD_PRIORITY_HIGHEST);
	m_bExitFlag = FALSE;

	return true;
}

// 释放串口
void CProcessCard::Release()
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
BOOL CProcessCard::SendData(BYTE* buf, int size)
{	
	/*int nWriteLog = GetPrivateProfileInt("CFG","WriteLog",0,"testBitmap.ini");
	if ( 1 == nWriteLog )
	{
	CString sLog;
	sLog.Format("-----------------------------------------------------CurB:%03d,F:%03d,E:%02d,V:%02d,N:%d",g_nAvg,g_nPeek,g_nExposure,buf[5],999);
	WriteLog(sLog);
	}*/
	;
	if (m_bExitFlag)
	{
		return FALSE;
	}
	DWORD dwWrite = 0;

	BOOL RTN = FALSE;
	try
	{
		RTN = WriteFile(m_hCom, (LPCVOID)buf, size, &dwWrite, NULL);
	}

	catch (CMemoryException* e)
	{
		
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}

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
UINT CProcessCard::RecvHandler(LPVOID lpParam)
{
	return ((CProcessCard*)(lpParam))->OnRecv(lpParam);
}
UINT CProcessCard::GetBytesInCOM()
{
	DWORD dwError = 0;  /** 错误码 */
	COMSTAT  comstat;   /** COMSTAT结构体,记录通信设备的状态信息 */
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;
	/** 在调用ReadFile和WriteFile之前,通过本函数清除以前遗留的错误标志 */
	if (ClearCommError(m_hCom, &dwError, &comstat))
	{
		BytesInQue = comstat.cbInQue; /** 获取在输入缓冲区中的字节数 */
	}

	return BytesInQue;
}

// 当接收到串口数据时
UINT CProcessCard::OnRecv(LPVOID lpParam)
{
	CProcessCard* pCom =(CProcessCard*)lpParam;
	DWORD dwSize=0;
	BOOL bRet = TRUE;
	//BYTE buf[32] = {0};
	//CString dbgbuf,dbgtemp;
	//char testdat[]="";
	while (1) 
	{	char testdata=0;
		dwSize = 0;
		UINT BytesInQue = pCom->GetBytesInCOM();
		do
		{
			bRet = ReadFile(m_hCom,			  // 串口句柄
						&m_rBuf, // 缓冲
						1,				  // 完成一次读串口操作需要读出的字节数总数
						&dwSize,	      // 完成一次读串口操作后实际读出的字节总数
						NULL );
					// 串口无效，关闭串口就退出了线程
			if(!bRet)
				return 0;
			if(dwSize > 0)
			{
				//cout<<m_rBuf;
			}


		}while(--BytesInQue);
		// 如果读到数据
	}
	return 1;
}

