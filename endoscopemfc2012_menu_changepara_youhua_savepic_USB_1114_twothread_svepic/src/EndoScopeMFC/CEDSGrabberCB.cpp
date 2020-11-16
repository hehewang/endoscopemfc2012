
#include "pch.h"
#include "CEDSGrabberCB.h"
#include "packQueue.h"
#include<iostream>
#include<time.h>
#include<fstream>
//#include<opencv2/xphoto/white_balance.hpp>
//#pragma comment(lib,"opencv_xphoto320.lib")

//CEvent g_videoevent(NULL,FALSE,FALSE,NULL);
//CEvent g_resizeevent(NULL,FALSE,FALSE,NULL);
//CEvent g_picevent(NULL,FALSE,FALSE,NULL);
//CEvent g_showevent(NULL,FALSE,FALSE,NULL);
//CEvent g_capevent(NULL,FALSE,FALSE,NULL);
CEDSGrabberCB::CEDSGrabberCB()
{
	g_bStop = FALSE;
	//m_bwritevideo = false;
	//m_bwritepicture = false;
	//m_badjwhite = false;
	//m_bwhiteok = false;
	//m_dZoom=1.0;
	//roi_w = IMGWIDTH;
	//roi_h = IMGHEIGHT;
	//roi_c=(roi_w-IMGWIDTH)/2;
	//roi_r=(roi_h-IMGHEIGHT)/2;
	//m_src = Mat(Size(IMGWIDTH, IMGHEIGHT), CV_8UC3);
	//m_lamda = new double[IMGSIZE];
	//m_dst=Mat(Size(1536,864),CV_8UC3);
	//IniWhilteLamda();
	buf=new BYTE[IMGWIDTH*IMGHEIGHT*3+1];
	//SetEvent(g_videoevent);
	//SetEvent(g_resizeevent);
	//SetEvent(g_capevent);
	//StartShowVideoThread();
}
//析构函数没用
CEDSGrabberCB::~CEDSGrabberCB()
{
	if(buf != NULL)
	{
		delete[] buf;
		buf = NULL;
	}
	//if (!m_pTaskList.empty())
	//{
	//	for (int i=0;i<m_pTaskList.size();++i)
	//	{
	//		BYTE* data = m_pTaskList.front();
	//		delete[]data;
	//		m_pTaskList.pop_front();
	//	}
	//	m_pTaskList.clear();
	//}
	//if (!m_pPicList.empty())
	//{
	//	for (int i=0;i<m_pPicList.size();++i)
	//	{
	//		BYTE* data = m_pPicList.front();
	//		delete[]data;
	//		m_pPicList.pop_front();
	//	}
	//	m_pPicList.clear();
	//}
	//if (!m_pShowList.empty())
	//{
	//	for (int i=0;i<m_pShowList.size();++i)
	//	{
	//		BYTE* data = m_pShowList.front();
	//		delete[]data;
	//		m_pShowList.pop_front();
	//	}
	//	m_pShowList.clear();
	//}

	//if (m_lamda)
	//{
	//	delete[]m_lamda;
	//	m_lamda = NULL;
	//}
	//if (!m_src.empty())
	//{
	//	m_src.release();
	//	delete buf;
	//}
	//m_bshowvideo = false;
}


STDMETHODIMP CEDSGrabberCB::QueryInterface(REFIID riid, void ** ppv)
{
	if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
		*ppv = (void *) static_cast<ISampleGrabberCB*> (this);
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP CEDSGrabberCB::SampleCB(double SampleTime, IMediaSample * pSample)
{
	return 0;
}
void CEDSGrabberCB::CropImage(BYTE* src,BYTE**dst)
{
	unsigned char *t_dst = *dst;
	memcpy(t_dst,src+883200,IMGWIDTH*IMGHEIGHT*3);
#if 0
	int pos=0,oldpos=0;

	for (int i = 0;i < IMGHEIGHT;++i)
	{
		for (int j = 0;j < IMGWIDTH;++j)
		{
			//printf("row:%d\n",i);
			//printf("col:%d\n",j);
			pos = i * IMGWIDTH * 3 + j * 3;
			oldpos=(i+230)*1280*3+(j)*3;//裁剪

			t_dst[pos + 0] = src[oldpos + 0];//(int)(src[pos + 0] * m_lamda[pos + 0]);

			t_dst[pos + 1] = src[oldpos + 1];
			t_dst[pos + 2] = src[oldpos + 2] ;//(int)(src[pos + 2] * m_lamda[pos + 2]);

		}
	}
#endif
}

// 重载回调函数BufferCB，用于保存图像
STDMETHODIMP CEDSGrabberCB::BufferCB(double dblSampleTime, BYTE * pBuffer, long lBufferSize)
{
	if (!pBuffer)
	{
		return E_POINTER;
	}
	if (g_bStop)
	{
		return 0;
	}
	/*Mat src;
	src=imread("d://test2.png");
	BYTE* pbuftest=new BYTE[2880*2160*3];
	memcpy(pbuftest,src.data,2880*2160*3);*/
	//BYTE* buf=new BYTE[1280*720*3];
	memset(buf,0,IMGWIDTH*IMGHEIGHT*3);
	CropImage(pBuffer,&buf);
	//WaitForSingleObject(g_capevent, INFINITE);
	//m_pShowList.push_back(buf);//操作队列
	WritePackQueue(buf,IMGWIDTH*IMGHEIGHT*3);
	//SetEvent(g_showevent);
	/*Mat tem=Mat(cvSize(2880,1530),CV_8UC3,buf);
	imshow("src",tem);
	waitKey(10);*/
	//计算白平衡参数
#if 0
	if (m_badjwhite)
	{
		m_bwhiteok = false;
		CalWhilteLamda(buf);
		return 0;
	}
	//白平衡调整
	AdjWhiteBalance(buf, IMGWIDTH, IMGHEIGHT, &m_src.data);

	//memcpy(m_src.data,pbuf,IMGSIZE);
	//grayWB->balanceWhite(m_src,m_src);
	if (m_src.empty())
	{
		return E_POINTER;
		//delete pbuftest;
		//delete buf;
	}
	//镜像处理
	flip(m_src, m_src, 0);
	//插值放大
	if (m_dZoom>1.0)
	{
		Mat t_img ;
		WaitForSingleObject(g_resizeevent, INFINITE);
		resize(m_src, t_img, cvSize(roi_w, roi_h), INTER_NEAREST);
		t_img(Rect(roi_c, roi_r, IMGWIDTH, IMGHEIGHT)).copyTo(m_src);
		SetEvent(g_resizeevent);
	}
	
	resize(m_src,m_dst,cvSize(1536,864),INTER_NEAREST);
	imshow("view", m_dst);
	//imwrite("test.jpg",m_dst);
	waitKey(1);
	g_FPS += 1;
	//录制视频
	if (m_bwritevideo)
	{
		unsigned char* pbuf = new unsigned char[IMGSIZE];
		memset(pbuf, 0, IMGSIZE);
		memcpy(pbuf, m_src.data, IMGSIZE);
		WaitForSingleObject(g_videoevent, INFINITE);
		m_pTaskList.push_back(pbuf);//操作队列
		SetEvent(g_videoevent);
	}
	if(m_bwritepicture)
	{
		unsigned char* pbuf = new unsigned char[IMGSIZE];
		memset(pbuf, 0, IMGSIZE);
		memcpy(pbuf, m_src.data, IMGSIZE);
	//	WaitForSingleObject(g_picevent, INFINITE);
		m_pPicList.push_back(pbuf);//操作队列
		SetEvent(g_picevent);
		m_bwritepicture = false;
	}
	//delete pbuftest;
			//delete buf;
#endif
	return 0;
}
/*
*功能：白平衡计算，先存储10张图片，然后计算均值，最后计算系数
*/
/*
void CEDSGrabberCB::CalWhilteLamda(BYTE * buf)
{
	//int pos = 0;
	imglistforwhite.push_back(buf);
	
	if(imglistforwhite.size()==10)
	{
		ofstream file("lamda.txt",'w');
		CString slamda;
		if (file.fail())
		{
			return;
		}
		fill(m_lamda, m_lamda +IMGSIZE,1);//初始化为1
		BYTE* data=(BYTE*)malloc(IMGSIZE);

		memset(data, 0, IMGSIZE);
		double* val=(double*)malloc(IMGSIZE*sizeof(double));
		memset(val,0,IMGSIZE*sizeof(double));
		int pos=0;
		//求和
		for (int i = 0; i < imglistforwhite.size(); i++)
		{
			BYTE* temp=imglistforwhite.front();
			imglistforwhite.pop_front();
			for (int i = 0;i < IMGHEIGHT;++i)
			{
				for (int j = 0;j < IMGWIDTH;++j)
				{
					pos=i * IMGWIDTH * 3 + j * 3;
					val[pos]+=temp[pos]/10.;
					val[pos+1]+=temp[pos+1]/10.;
					val[pos+2]+=temp[pos+2]/10.;
				}
			}
		}
		for (int i = 0;i < IMGHEIGHT;++i)
		{
			for (int j = 0;j < IMGWIDTH;++j)
			{
				int pos=i * IMGWIDTH * 3 + j * 3;
				data[pos]=(int)val[pos];
				data[pos+1]=(int)val[pos+1];
				data[pos+2]=(int)val[pos+2];
			}

		}
		//计算系数
		Mat src=Mat(IMGHEIGHT,IMGWIDTH,CV_8UC3,data);
		Mat dst;
		int pos5;
		double b=255, g=255, r=255;
		blur(src,dst,Size(3,3));
		for (int i = 0;i < IMGHEIGHT;++i)
		{
			for (int j = 0;j < IMGWIDTH;++j)
			{

				pos5=i * IMGWIDTH * 3 + j * 3;
				b=dst.at<Vec3b>(i,j)[0];
				g=dst.at<Vec3b>(i,j)[1];
				r=dst.at<Vec3b>(i,j)[2];
				//if (0 == b)
				//	b = 1;
				if (0 == g)
					g= 1;
				if(0==r)
					r=1;
				m_lamda[pos5] = 1.0;//(double)buf[pos + 1] / (double)buf[pos + 0];//Lamda_B=G/B
				m_lamda[pos5 + 1]= r/ g;//Lamda_G=R/G
				m_lamda[pos5 + 2] = 1.0;//(double)buf[pos + 1] / (double)buf[pos + 2];//Lamda_R=G/R

				slamda="";
				slamda.Format(_T("%4lf\n%.4lf\n%.4lf\n"),m_lamda[pos5 + 2],m_lamda[pos5 + 1],m_lamda[pos5]);
				file<<slamda;
			}
		}
		delete data;
		imglistforwhite.clear();
		m_badjwhite = false;
		file.close();
		//提醒白平衡完成
		m_bwhiteok = true;
	}

}
*/
/*void CEDSGrabberCB::CalWhilteLamda(BYTE * buf,int masksize)
{
	//int pos = 0;
	ofstream file("lamda.txt",'w');
	CString slamda;
	if (file.fail())
	{
		return;
	}
	fill(m_lamda, m_lamda +IMGSIZE,1);//初始化为1
	int pos[9]={0};
	BYTE* data=(BYTE*)malloc(IMGSIZE);
	memset(data, 0, IMGSIZE);
	memcpy(data,buf, IMGSIZE);
	for (int i = 0;i < IMGHEIGHT;++i)
	{
		for (int j = 0;j < IMGWIDTH;++j)
		{
			pos[0]=(i-1) * IMGWIDTH * 3 + (j-1) * 3;
			pos[1]=(i-1) * IMGWIDTH * 3 + j * 3;
			pos[2]=(i-1) * IMGWIDTH * 3 + (j+1) * 3;
			pos[3]=i * IMGWIDTH * 3 + (j-1) * 3;
			pos[4]=i * IMGWIDTH * 3 + j * 3;
			pos[5]=i * IMGWIDTH * 3 + (j+1) * 3;
			pos[6]=(i+1) * IMGWIDTH * 3 + (j-1) * 3;
			pos[7]=(i+1) * IMGWIDTH * 3 + j * 3;
			pos[8]=(i+1) * IMGWIDTH * 3 + (j+1) * 3;
			if(i<1||i>(IMGHEIGHT-1)||j<1||j>(IMGWIDTH))//第一行
			{
				data[pos[4]]=data[pos[4]];
				data[pos[4]+1]=data[pos[4]+1];
				data[pos[4]+2]=data[pos[4]+2];
			}
			else
			{
				data[pos[4]]=(int)((data[pos[0]]+data[pos[2]]+data[pos[4]]+data[pos[6]]+data[pos[8]])/5.);
				data[pos[4]+1]=(int)((data[pos[0]+1]+data[pos[2]+1]+data[pos[4]+1]+data[pos[6]+1]+data[pos[8]+1])/5.);
				data[pos[4]+2]=(int)((data[pos[0]+2]+data[pos[2]+2]+data[pos[4]+2]+data[pos[6]+2]+data[pos[8]+2])/5.);
			}
			
		}
	}
	Mat src=Mat(IMGHEIGHT,IMGWIDTH,CV_8UC3,data);
	Mat dst;
	blur(src,dst,Size(9,9));
	for (int i = 0;i < IMGHEIGHT;++i)
	{
		for (int j = 0;j < IMGWIDTH;++j)
		{
			int pos5=i * IMGWIDTH * 3 + j * 3;
			double b=dst.at<Vec3b>(i,j)[0];
			double g=dst.at<Vec3b>(i,j)[1];
			double r=dst.at<Vec3b>(i,j)[2];
			if (0 == b)
				b = 1;
			if (0 == g)
				g= 1;
			if(0==r)
				r=1;
			//m_lamda[pos + 0] = (double)buf[pos + 1] / (double)buf[pos + 0];//Lamda_B=G/B
			//m_lamda[pos + 2] = (double)buf[pos + 1] / (double)buf[pos + 2];//Lamda_R=G/R
			//m_lamda[pos + 1] = 1.0;//Lamda_G=G/G
			//2020.4.23-LIJIAN
			m_lamda[pos5 + 1]= r/ g;//Lamda_G=R/G
			m_lamda[pos5] = 1.0;//(double)buf[pos + 1] / (double)buf[pos + 0];//Lamda_B=G/B
			m_lamda[pos5 + 2] = 1.0;//(double)buf[pos + 1] / (double)buf[pos + 2];//Lamda_R=G/R

			slamda="";
			slamda.Format(_T("%4lf\n%.4lf\n%.4lf\n"),m_lamda[pos5 + 2],m_lamda[pos5 + 1],m_lamda[pos5]);
			file<<slamda;
		}
	}
	delete data;
	m_badjwhite = false;
	file.close();

}*/
// 创建Ipl图像对象
#if 0
extern int g_nThresh;
extern int g_nThreshNew;
extern int g_nChangePixel;
extern int g_nChange255;
void CEDSGrabberCB::AdjWhiteBalance(unsigned char* src, int width, int height, unsigned char** dst)
{
	
	int pos = 0,oldpos=0;
	unsigned char *t_dst = *dst;

	for (int i = 0;i < height;++i)
	{
		for (int j = 0;j < width;++j)
		{
			pos = i * width * 3 + j * 3;
			//oldpos=(i+80)*1920*3+(j+290)*3;//裁剪
			/*
			double yuu_y=0.3*src[pos]+0.59*src[pos+1]+0.11*src[pos+2];
			if (yuu_y>g_thred)
			{
			t_dst[pos + 0]=src[pos];
			t_dst[pos+1]=src[pos+1];
			t_dst[pos+2]=src[pos+2];
			continue;
			}
			*/
#if 0 //wm_20200919
			if(3 == g_nChangePixel)
			{
				if((src[pos + 0] >= g_nThresh)&&(src[pos + 1] >= g_nThresh)&&(src[pos + 2] >= g_nThresh))
				{
					t_dst[pos + 0] = src[pos + 0];
					t_dst[pos + 1] = src[pos + 1];
					t_dst[pos + 2] = src[pos + 2];
					continue;
				}
			}
			if(2 == g_nChangePixel)
			{
				if(((src[pos + 0] >= g_nThresh)&&(src[pos + 1] >= g_nThresh))||((src[pos + 1] >= g_nThresh)&&(src[pos + 2] >= g_nThresh))\
					||(src[pos + 0] >= g_nThresh)&&(src[pos + 2] >= g_nThresh))
				{
					t_dst[pos + 0] = src[pos + 0];
					t_dst[pos + 1] = src[pos + 1];
					t_dst[pos + 2] = src[pos + 2];
					continue;
				}
			}
			if(1 == g_nChangePixel)
			{
				if((src[pos] >= g_nThresh) || (src[pos + 1] >= g_nThresh) || (src[pos + 2] >= g_nThresh))
				{
					t_dst[pos + 0] = src[pos + 0];
					t_dst[pos + 1] = src[pos + 1];
					t_dst[pos + 2] = src[pos + 2];
					continue;
				}
			}
#endif
			t_dst[pos + 0] = src[pos + 0];//(int)(src[pos + 0] * m_lamda[pos + 0]);

			t_dst[pos + 1] = (int)(src[pos + 1] * m_lamda[pos + 1]);
			t_dst[pos + 2] = src[pos + 2];//(int)(src[pos + 2] * m_lamda[pos + 2]);
#if 0
			if(1 == g_nChange255)
			{
				if (t_dst[pos + 0] >= g_nThreshNew)
				{
					t_dst[pos +2] = 255;
					t_dst[pos + 0] = 255;
					t_dst[pos + 1] = 255;
				}
				if (t_dst[pos + 1] >= g_nThreshNew)
				{
					t_dst[pos + 2] = 255;
					t_dst[pos + 0] = 255;
					t_dst[pos + 1] = 255;
				}
				if (t_dst[pos + 2] >= g_nThreshNew)
				{
					t_dst[pos + 2] = 255;
					t_dst[pos + 0] = 255;
					t_dst[pos + 1] = 255;
				}
			}
#endif
		}
	}
}
#endif
//void CEDSGrabberCB::IniWhilteLamda()
//{
//	ifstream file("lamda.txt");
//	int i=0;
//	if (file.fail())
//	{
//		fill(m_lamda, m_lamda +IMGSIZE,1);
//		return;
//	}
//	else
//	{
//		string t_buf;
//		while(!file.eof())
//		{
//			getline(file,t_buf);
//			m_lamda[i]=atof(t_buf.c_str());
//			i++;
//		}
//		file.clear();
//		file.close();
//	}
//
//}
STDMETHODIMP_(ULONG) CEDSGrabberCB::AddRef()
{
	return 2;
}

STDMETHODIMP_(ULONG) CEDSGrabberCB::Release()
{
	return 1;
}

//bool CEDSGrabberCB::StartShowVideoThread()
//{
//	m_bshowvideo = true;
//	CWinThread *pThread = AfxBeginThread(ShowVideoThreadFunc,this);
//	if(NULL == pThread)
//		return false;
//	return true;
//}

