#pragma once
#include <directshow/DShow.h>
#include <directshow/qedit.h>
#include <list>
#include <array>
#include<streams.h>
#include<opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include<opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include<opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <opencv2/core/types_c.h>

#pragma comment(lib, "opencv_core320.lib")
#pragma comment(lib, "opencv_highgui320.lib")
#pragma comment(lib, "opencv_imgcodecs320.lib")
#pragma comment(lib, "opencv_imgproc320.lib")
#pragma comment(lib, "opencv_video320.lib")
#pragma comment(lib, "opencv_videoio320.lib")
extern CEvent g_videoevent;
extern CEvent g_resizeevent;
extern CEvent g_picevent;
using namespace cv;
class CEDSGrabberCB : public ISampleGrabberCB
{
public:
	long lWidth;	// 图像宽
	long lHeight;	// 图像高
	long m_nSize;	// 所占字节数
	//int roi_w ;
	//int roi_h;
	//int roi_c ;
	//int roi_r ;
	//Mat m_dst;
	bool m_bwhiteok;//白平衡是否矫正完成
	//double m_dZoom;    //放大倍数
	//Mat m_src;//数据
	BYTE* buf;
	//std::list<BYTE*> m_pTaskList;//处理任务队列
	//std::list<BYTE*> m_pPicList;//处理图片队列
	//std::list<BYTE*> m_pShowList;//显示队列
	//bool m_bwritevideo;
	//bool m_bwritepicture;
	bool m_badjwhite;//是否调整白平衡
	//bool m_bshowvideo;//显示标志
	//double*m_lamda;//白平衡系数
	CEDSGrabberCB();
	~CEDSGrabberCB();
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);	// 查询接口
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample * pSample);	// 返回media Sample指针的回调，未用
	STDMETHODIMP BufferCB(double dblSampleTime, BYTE * pBuffer, long lBufferSize);	// 返回Sample buffer指针的回调，使用	
	//void AdjWhiteBalance(unsigned char* src, int width, int height, unsigned char** dst);
	//void CalWhilteLamda(BYTE * buf);
	//void IniWhilteLamda();
	void CropImage(BYTE* src,BYTE**dst);
	//bool StartShowVideoThread();

};