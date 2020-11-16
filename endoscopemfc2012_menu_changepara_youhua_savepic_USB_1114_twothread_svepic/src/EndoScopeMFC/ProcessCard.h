#pragma once
// VideoCOM串口通信类

#include <time.h>
#include "datatype.h"
class CProcessCard
{
public:
	CProcessCard(void);
	~CProcessCard(void);

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
	BYTE m_rBuf[256];

	CComDataHandler* m_pDataHandler;
private:
	UINT GetBytesInCOM();
};

