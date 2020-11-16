/*
* COM串口操作
* 文件类型: .h文件
*/
#ifndef __VIDEOCOMIO_H__
#define __VIDEOCOMIO_H__

// VideoCOM串口通信类

#include <time.h>
#include "datatype.h"


unsigned short GetCrc(const unsigned char *pszbuf,int len);

class CVideoComIO 
{
public:
	CVideoComIO();
	virtual ~CVideoComIO();

public:

	// 初始化COM
	bool Initialize(CString comName, int nBaudRate);

	// 释放COM
	void Release();

	// 数据发送
	BOOL SendData(BYTE* buf, int size);

	// 串口接收数据监控线程
	static UINT RecvHandler( LPVOID lpParam );

	// 当接收到串口数据时
	UINT OnRecv(LPVOID lpParam);

	//线程退出标识
	bool m_bExitFlag;

	void SetDataHandler( CComDataHandler* p )
	{
		m_pDataHandler = p;
	}

protected:
	HANDLE m_hCom;   // 串口句柄
	CWinThread* m_comThread;
	int m_nBaudRate; // 比特率

	// 已经读取的数据个数
	DWORD m_rSize;

	// 读缓冲
	BYTE m_rBuf[32];

	CComDataHandler* m_pDataHandler;
};

#endif
