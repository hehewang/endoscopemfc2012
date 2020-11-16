#pragma once

#include <iostream>

#include "afxstr.h"
#include <afxwin.h>
#include<vector>
#include "CEDSGrabberCB.h"
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if(NULL!=x){x->Release();x=NULL;}
#endif



//extern CCritSec g_dVideoLock;//录像缓存队列锁
// CEDSGrabberCB，此类中需要重载几个函数，这是回调函数


// CEDSCapture
class CEDSCapture : public CWnd
{
	DECLARE_DYNAMIC(CEDSCapture)
//friend class CEDSGrabberCB;
public:
	//void GrabOneFrame(BOOL bGrab);
	HRESULT Init(int iDeviceID,HWND hWnd);	// 对类做一些DW视频初始化工作
	void Pause();			// 暂停视频捕捉
	void Run();		// 视频捕捉
	void Stop();
	void GrabImage( CString );// 捕捉单帧视频图像
	void VideoAdjust( int ntype);// 视频增强处理
	void DeNoise();	// 视频降噪
	void Sharped();	// 视频锐化
	//void GetScale(int nw);	// 计算视频与窗口尺寸比例
	//int EnumDevices(HWND hList);
	void StartWriteVideo();
	void StopWriteVideo();
	void StartWritePic();
	void StopWritePic();
	BYTE* GetFrame();
	BYTE* GetPicture();
	void SetupLamdaRGB();
	void SetZoom(double flag);
	bool GetStateWhiteBalance();
	CEDSCapture();
	virtual ~CEDSCapture();


public:
	float m_fscale_x,m_fscale_y;	// 视频与窗口尺寸比例
	std::vector<IBaseFilter *> m_vecDevices;
	BYTE* m_pBuffer;//帧 数据
	int m_nWidth, m_nHeight;//视频宽高
private:
	// 枚举视频运行状态
	enum PLAYSTATE {
		Stopped, 
		Paused, 
		Running, 
		Init_Ready
	};
	enum PLAYSTATE m_psCurrent;
	HWND m_hApp;
	DWORD m_dwGraphRegister;
	BOOL m_bOneShot;
	CEDSGrabberCB g_CB;	// 此对象进行视频回调处理
	IVideoWindow  *m_pVW;
	IMediaControl *m_pMC;
	IMediaEventEx *m_pME;
	IGraphBuilder *m_pGraph;
	ICaptureGraphBuilder2 *m_pCapture;
	CComPtr <ISampleGrabber> m_pGrabber;
protected:
	HRESULT GetInterfaces(void);		// 创建并获取DW接口
	HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter);	// 寻找捕捉设备
	HRESULT SetupVideoWindow(void);		// 设置视频窗口
	HRESULT ChangePreviewState(int nShow);
	HRESULT HandleGraphEvent(void);

	void Msg(TCHAR *szFormat, ...);
	void CloseInterfaces(void);			// 关闭接口
	void ResizeVideoWindow(void);		// 调整视频窗口大小
	void FreeMediaType(AM_MEDIA_TYPE& mt);	// 释放sample变量

	DECLARE_MESSAGE_MAP()
};




