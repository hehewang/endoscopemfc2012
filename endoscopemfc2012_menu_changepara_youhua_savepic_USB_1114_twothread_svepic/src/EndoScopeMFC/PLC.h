#pragma once
#include "DataType.h"
#include "VideoComIO.h"
#include "LightComIO.h"
#include "ProcessCard.h"
#include <iostream>

#define WM_SERIALPORT_MSG		WM_USER+100


class CPLC :
	public CComDataHandler
{
public:
	CPLC(void);
	virtual ~CPLC(void);
private:
	bool m_bRegister;				//收到下位机第一次心跳回复

	CString m_strSemState;		    //半导体状态
public:
	//视频串口初始化
	bool VideoInitialize( CString strCOM, int nBaudRate );

	//冷光源串口初始化
	bool LightInitialize( CString strCOM, int nBaudRate );

	//图像处理板串口初始化
	bool ProcessCardInitialize(CString strCOM, int nBaudRate );

	//处理接收到的串口数据
	void ProcessData( unsigned char* pBuf, int size );

	//获取与下位机连接状态
	bool GetRegState(){	return m_bRegister; }

	//获取\设置半导体状态
	CString GetSemState(){ return m_strSemState; }
	void SetSemState( CString strSem ){ m_strSemState = strSem; }

	//心跳
	BOOL KeepAlive()
	{
		BYTE bBuf[2] = { 0x06, 0xA1 };
		return m_VideoComIO.SendData(bBuf,2);
	}

	//开启蜂鸣器
	BOOL OpenBuzzer()
	{
		BYTE bBuf[3] = { 0x07, 0xB1, 0x00 };
		return m_VideoComIO.SendData( bBuf, 3 );
	}
	////内镜型号
	//BOOL EndoscopicModel()
	//{
	//	//BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x31, 0x00, 0x01};
	//	return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	//}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////录入病人信息
	//正在录入
	BOOL NowEnter()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x31, 0x00, 0x01};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//录入完成
	BOOL OverEnter()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x31, 0x00, 0x00};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//准备就绪
	BOOL ReadyEnter()
	{
		BYTE bBuf[ 14 ] = {0x0D, 0x82, 0x00, 0x31, 0xD7, 0xBC, 0xB1, 0xB8, 0xBE, 0xCD, 0xD0, 0xF8, 0xFF, 0xFF};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////录像
	//正在录制
	BOOL NowVideo()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x33, 0x00, 0x03};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//停止录制
	BOOL StopVideo()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x33, 0x00, 0x02};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//准备录制
	BOOL ReadyVideo()
	{
		BYTE bBuf[ 14 ] = {0x0D, 0x82, 0x00, 0x41, 0xD7, 0xBC, 0xB1, 0xB8, 0xBE, 0xCD, 0xD0, 0xF8, 0xFF, 0xFF};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////存储到外部设备
	//正在存储
	BOOL NowSave()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x35, 0x00, 0x05};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//停止存储
	BOOL StopSave()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x35, 0x00, 0x04};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//存储完成
	BOOL OverSave()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x35, 0x00, 0x04};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//未插入外设
	BOOL NotLink()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x35, 0x00, 0x06};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//外设已插入
	BOOL AlreadyLink()
	{
		BYTE bBuf[ 16 ] = {0x0F, 0x82, 0x00, 0x51, 0xCD, 0xE2, 0xC9, 0xE8, 0xD2, 0xD1, 0xB2, 0xE5, 0xC8, 0xEB, 0xFF, 0xFF};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//外设已插入
	BOOL ReadySave()
	{
		BYTE bBuf[ 14 ] = {0x0D, 0x82, 0x00, 0x51, 0xD7, 0xBC, 0xB1, 0xB8, 0xBE, 0xCD, 0xD0, 0xF8, 0xFF, 0xFF};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////打印
	//正在打印
	BOOL NowPrint()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x37, 0x00, 0x08};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//停止打印
	BOOL StopPrint()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x37, 0x00, 0x07};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//打印完成
	BOOL OverPrint()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x37, 0x00, 0x07};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//打印机未连接
	BOOL NotLinkPrint()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x37, 0x00, 0x09};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//准备就绪
	BOOL ReadyPrint()
	{
		BYTE bBuf[ 14 ] = {0x0D, 0x82, 0x00, 0x61, 0xD7, 0xBC, 0xB1, 0xB8, 0xBE, 0xCD, 0xD0, 0xF8, 0xFF, 0xFF};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////测光模式
	//平均测光
	BOOL AverageMetering()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x02, 0x00, 0x00};
		 m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
		BYTE Buf[15]={0xFF,0xAA,0x03,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData(Buf,sizeof(Buf)/sizeof(Buf[0]));
	}
	//峰值测光
	BOOL PeakMetering()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x02, 0x00, 0x01};
		 m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
		BYTE Buf[15]={0xFF,0xAA,0x03,0x03,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData(Buf,sizeof(Buf)/sizeof(Buf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////B值调节
	BOOL B( int nVal )
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x91, 0x00, 0x00};
		BYTE buf[15] = { 0xFF,0xAA,0x03,0x02,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		//int temp = nVal + 5;
		buf[7] =(BYTE)(nVal+0x37);
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		if ( nVal >= 0 )
		{
			bBuf[5] = (BYTE)nVal;
		}
		else
		{
			bBuf[4] = 0xFF;
			bBuf[5] = (BYTE)(0xFF + nVal + 1);
		}
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////R值调节
	BOOL R( int nVal )
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x95, 0x00, 0x00};
		BYTE buf[15] = { 0xFF,0xAA,0x03,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		//int temp = nVal + 5;
		buf[7] = (BYTE)(nVal+0x37);
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		if ( nVal >= 0 )
		{
			bBuf[5] = (BYTE)nVal;
		}
		else
		{
			bBuf[4] = 0xFF;
			bBuf[5] = (BYTE)(0xFF + nVal + 1);
		}
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////图像放大
	BOOL Zoom_Card1()//放大1倍
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	BOOL Zoom_Card2()//放大2倍
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x06, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//放大1倍
	BOOL Zoom1()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x11, 0x01, 0x00, 0x0A};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//放大1.2倍
	BOOL Zoom2()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x11, 0x01, 0x00, 0x0C};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//放大1.5倍
	BOOL Zoom3()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x11, 0x01, 0x00, 0x0F};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//放大2倍
	BOOL Zoom4()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x11, 0x01, 0x00, 0x14};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////轮廓设定
	//设置圆角
	BOOL SetCircular()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x06, 0x00, 0x00};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//设置方角
	BOOL SetSquare()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x06, 0x00, 0x01};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////冷光源调节
	//冷光源开
	BOOL OpenLight()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x71, 0x00, 0x01};
#ifdef _LPort
		return m_LightComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
#else
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
#endif
	}
	//冷光源关
	BOOL CloseLight()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x71, 0x00, 0x00};
#ifdef _LPort
		return m_LightComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
#else
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
#endif
	}
	BOOL Light( int nVal )
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x21, 0x00, 0x00};
		bBuf[5] = (BYTE)nVal;
#ifdef _LPort
		return m_LightComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
#else
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
#endif
	}
	//光源自动开
	BOOL OpenAuto()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x25, 0x00, 0x01};
#ifdef _LPort
		return m_LightComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
#else
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
#endif
	}
	//光源自动关
	BOOL CloseAuto()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0x25, 0x00, 0x00};
#ifdef _LPort
		return m_LightComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
#else
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
#endif
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////清晰度调节
	BOOL Clarity( int nVal )
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA0, 0x00, 0x00};
		bBuf[5] = (BYTE)nVal;
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////亮度调节
	//亮度0档
	BOOL Brightness0()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x00};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));

		Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度1档
	BOOL Brightness1()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x01};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x3D,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);

		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度2档
	BOOL Brightness2()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x02};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度3档
	BOOL Brightness3()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x03};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);

		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度4档
	BOOL Brightness4()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x04};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度5档
	BOOL Brightness5()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x05};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度6档
	BOOL Brightness6()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x06};

		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度7档
	BOOL Brightness7()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x07};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x43,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		 return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度8档
	BOOL Brightness8()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x08};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度9档
	BOOL Brightness9()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x09};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x45,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度10档
	BOOL Brightness10()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0x00, 0x0A};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x46,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度-1档
	BOOL Brightness11()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0xFF, 0xFF};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x3B,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		return m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		 m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度-2档
	BOOL Brightness12()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0xFF, 0xFE};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x3A,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		return m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		 m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度-3档
	BOOL Brightness13()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0xFF, 0xFD};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x39,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度-4档
	BOOL Brightness14()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0xFF, 0xFC};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度-5档
	BOOL Brightness15()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0xFF, 0xFB};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x37,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度-6档
	BOOL Brightness16()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0xFF, 0xFA};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度-7档
	BOOL Brightness17()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0xFF, 0xF9};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x35,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度-8档
	BOOL Brightness18()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0xFF, 0xF8};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x34,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度-9档
	BOOL Brightness19()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0xFF, 0xF7};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//亮度-10档
	BOOL Brightness20()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA1, 0xFF, 0xF6};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////对比度调节
	BOOL Contrast( int nVal )
	{
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0D, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = (BYTE)(nVal+5);
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA2, 0x00, 0x00};
		bBuf[5] = (BYTE)nVal;
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////饱和度调节
	BOOL Saturation( int nVal )
	{
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x02, 0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		//buf[7] = nVal*10;
		int temp = nVal+60;
		buf[7] = (BYTE)temp;
		m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		Sleep(5);
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA3, 0x00, 0x00};
		bBuf[5] = (BYTE)nVal;
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////曝光调节
	//曝光0档
	BOOL Exposure0()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0x00, 0x00};
		//BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x0C, 0x00,0x00,0x00,0x64,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//曝光-1档
	BOOL Exposure1()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0xFF, 0xFF};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0x01;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//曝光-2档
	BOOL Exposure2()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0xFF, 0xFE};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0x02;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//曝光-3档
	BOOL Exposure3()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0xFF, 0xFD};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0x03;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//曝光-4档
	BOOL Exposure4()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0xFF, 0xFC};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0x04;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//曝光-5档
	BOOL Exposure5()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0xFF, 0xFB};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0x05;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//曝光-6档
	BOOL Exposure6()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0xFF, 0xFA};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0x06;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//曝光-7档
	BOOL Exposure7()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0xFF, 0xF9};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0x07;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//曝光-8档
	BOOL Exposure8()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0xFF, 0xF8};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0x08;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//曝光-9档
	BOOL Exposure9()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0xFF, 0xF7};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0x09;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//曝光-10档
	BOOL Exposure10()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA4, 0xFF, 0xF6};
		BYTE buf[15] = { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = 0x0A;
		 m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));
		 Sleep(5);
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//锐度调节，范围0-10
	BOOL Sharpness(int val)
	{
		BYTE bBuf[8] = { 0xA5, 0x5A, 0x05, 0x82, 0x00, 0xA0, 0x00, 0x00};
		bBuf[7] = val;
		m_VideoComIO.SendData(bBuf, sizeof(bBuf) / sizeof(bBuf[0]));
		Sleep(5);
		BYTE buf[15]= { 0xFF, 0xAA, 0x03, 0x05, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		buf[7] = val;
		return m_ProcessCardComIO.SendData(buf, sizeof(buf) / sizeof(buf[0]));

	}
	//自动曝光开
	BOOL ExposureOpen()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA6, 0x00, 00};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//自动曝光开
	BOOL ExposureClose()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA6, 0x00, 01};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////增益调节
	BOOL Gain( int nVal )
	{
		//std::cout<<"Gain:"<<nVal<<endl;
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA5, 0x00, 0x00};
		bBuf[5] = (BYTE)nVal;
		 m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
		 Sleep(5);
		BYTE buf[ 15 ] = {0xFF, 0xAA, 0x03, 0x07, 0x00, 0x00, 0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		buf[7] = (BYTE)nVal;
		return m_ProcessCardComIO.SendData( buf, sizeof(buf)/sizeof(buf[0]));
	}
	//自动增益开
	BOOL GainOpen()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA7, 0x00, 0x00};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	//自动增益关
	BOOL GainClose()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x82, 0x00, 0xA7, 0x00, 0x01};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////主控板应答
	//应答
	BOOL Recv()
	{
		BYTE bBuf[ 4 ] = {0x03, 0x82, 0x00, 0x5F};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////获取内镜信息
	BOOL GetInfo()
	{
		BYTE bBuf[ 6 ] = {0x05, 0x83, 0x00, 0x55, 0x00, 0xF0};
		return m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////设置白平衡
	BOOL WhiteBalance()
	{
		//BYTE bBuf[ 7 ] = {0x06, 0x83, 0x00, 0x24, 0x01, 0x00, 0x08};
		BYTE cBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x0A, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		//AfxMessageBox("updatewhite");
		BOOL ok=m_ProcessCardComIO.SendData( cBuf, sizeof(cBuf)/sizeof(cBuf[0]));
		//BOOL ok2=m_VideoComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
		return ok;//&ok2;
	}

	////////////////////////////////////////////////////////////////////////
	BOOL Gain1()
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x07, 0x00, 0x00, 0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	////////////////////////////////////////////////////////////////////////
	BOOL Gain5()
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x07, 0x00, 0x00, 0x00,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	BOOL Gain15()
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x07, 0x00, 0x00, 0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}

	BOOL MenuStart()
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x10, 0x00, 0x01, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	BOOL MenuStop()
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x10, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	BOOL MenuUp()
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x11, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	BOOL MenuDown()
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x12, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	BOOL MenuLeft()
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x13, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	BOOL MenuRight()
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x14, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}
	BOOL MenuEnter()
	{
		BYTE bBuf[ 15 ] = {0xFF, 0xAA, 0x03, 0x15, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		return m_ProcessCardComIO.SendData( bBuf, sizeof(bBuf)/sizeof(bBuf[0]));
	}

	//获得显示窗口指针
	void GetShowDlgPointer( CWnd *pShowDlg );

	//串口对象
	CVideoComIO m_VideoComIO;
	CLightComIO m_LightComIO;
	CProcessCard m_ProcessCardComIO;
	
};


extern CPLC *g_pPLC;


