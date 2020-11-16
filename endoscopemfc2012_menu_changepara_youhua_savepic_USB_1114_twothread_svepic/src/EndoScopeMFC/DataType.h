#pragma once

#include <time.h>
#include <sys/timeb.h>
#include <fstream>
#pragma warning(disable:4786)//去掉使用map时的一堆告警
#include <map>
#include <queue>
#include <list>

#include <string>

using std::string;

using namespace std;

# define IMGWIDTH 1280//原视频宽
# define IMGHEIGHT 720//原视频高
# define IMGSIZE IMGWIDTH*IMGHEIGHT*3//2764800 //整个视频尺寸720P 2764800   1080P 6220800

#define SAFE_DELETE( ptr ) if ((ptr) != NULL) { delete (ptr); (ptr) = NULL; }//delete指针
#define SAFE_DELETE_ARRAY( ptr ) if ((ptr) != NULL) { delete[] (ptr); (ptr) = NULL; }//delete数组指针

//清晰度
typedef enum {
	SHARPNESS0,
	SHARPNESS1,
	SHARPNESS2,
	SHARPNESS3,
	SHARPNESS4,
	SHARPNESS5,
	SHARPNESS6,
	SHARPNESS7,
	SHARPNESS8,
	SHARPNESS9,
	SHARPNESS10
} VIDEO_SHARPNESS;

//图像亮度
typedef enum {
	BRIGHTNESS0,
	BRIGHTNESS1,
	BRIGHTNESS2,
	BRIGHTNESS3,
	BRIGHTNESS4,
	BRIGHTNESS5,
	BRIGHTNESS6,
	BRIGHTNESS7,
	BRIGHTNESS8,
	BRIGHTNESS9,
	BRIGHTNESS10
} VIDEO_BRIGHTNESS;

typedef struct picZOOMSTRUCT
{
	int xSrc;
	int ySrc;
	int SrcWidth;
	int SrcHeight;
	int nCut;
	int nCutRect;
}ZOOMSTRUCT;

//对比度
typedef enum {
	Contrast0,
	Contrast1,
	Contrast2,
	Contrast3,
	Contrast4,
	Contrast5,
	Contrast6,
	Contrast7,
	Contrast8,
	Contrast9,
	Contrast10
} VIDEO_CONTRAST;

//饱和度
typedef enum {
	SATURATION0,
	SATURATION1,
	SATURATION2,
	SATURATION3,
	SATURATION4,
	SATURATION5,
	SATURATION6,
	SATURATION7,
	SATURATION8,
	SATURATION9,
	SATURATION10
} VIDEO_SATURATION;

//曝光
typedef enum {
	EXPOSURE0,
	EXPOSURE1,
	EXPOSURE2,
	EXPOSURE3,
	EXPOSURE4,
	EXPOSURE5,
	EXPOSURE6,
	EXPOSURE7,
	EXPOSURE8,
	EXPOSURE9,
	EXPOSURE10
} VIDEO_EXPOSURE;

//增益
typedef enum {
	GAIN0,
	GAIN1,
	GAIN2,
	GAIN3,
	GAIN4,
	GAIN5,
	GAIN6,
	GAIN7,
	GAIN8,
	GAIN9,
	GAIN10
} VIDEO_GAIN;

//冷光源亮度
typedef enum {
	LGAIN0,
	LGAIN1,
	LGAIN2,
	LGAIN3,
	LGAIN4,
	LGAIN5,
	LGAIN6,
	LGAIN7,
	LGAIN8,
	LGAIN9,
	LGAIN10
} LIGHT_BRIGHTNESS;

//图像放大
typedef enum {
	ZOOM1,
	ZOOM2,
	ZOOM3,
	ZOOM4,
} VIDEO_ZOOM;

//RED调节
typedef enum {
	RED0,
	RED1,
	RED2,
	RED3,
	RED4,
	RED5,
	RED6,
	RED7,
	RED8,
	RED9,
	RED10,
} VIDEO_RED;

//BLUE调节
typedef enum {
	BLUE0,
	BLUE1,
	BLUE2,
	BLUE3,
	BLUE4,
	BLUE5,
	BLUE6,
	BLUE7,
	BLUE8,
	BLUE9,
	BLUE10,
} VIDEO_BLUE;

//R、B标识
enum {
	RED_SIGN,
	BLUE_SIGN
};
extern ZOOMSTRUCT picZoom;
extern HANDLE g_hPic2VideoThread;
extern HANDLE g_hSaveVideoThread;
extern HANDLE g_hSaveDataThread;
extern int g_FPS;//粗略统计帧率
extern int g_thred;
extern int g_nSharpness;	//清晰度
extern int g_nBrightness;	//图像亮度
extern int g_nContrast;		//对比度
extern int g_nSaturation;	//饱和度
extern int g_nExposure;		//曝光
extern int g_nGain;			//增益
extern int g_nWhitebalance; //白平衡
extern int g_nLBrightness;	//冷光源亮度

extern int g_nRed;			//R调节
extern int g_nBlue;			//B调节
extern int g_nMetering;		//测光模式 0:平均 1:峰值
extern int g_nPic;			//存储照片数
extern int g_nShowShape;	//显示 0:方角 1:圆角
extern int g_nPeek;			//测光峰值
extern int g_ncout;//LJ731
extern int g_nAvg;			//测光均值
extern int g_nNBI;			//NBI
extern int g_nVCOM;			//视频串口
extern int g_nLCOM;			//冷光源串口
extern int g_nCCOM;         //图像处理板串口
extern int g_nPort;			//port
extern double g_lamdar;   //R的系数
extern double g_lamdab;
extern double g_lamdag;
extern double g_fGainR;		//白平衡R值
extern double g_fGainB;		//白平衡B值
extern int g_iMenu;			//菜单

extern BOOL g_bFICE;		//FICE
extern BOOL g_bExposure;	//自动曝光
extern BOOL g_bGain;		//自动增益
extern BOOL g_bWhiteBalance;//自动白平衡
extern BOOL g_bIsLightOpen;	//冷光源开关
extern BOOL g_nOneSave;		//保存一次图片
extern BOOL g_bIsUpan;		//u盘插入标记
extern BOOL g_bIsAutoLightOpen;		//自动调节冷光源
extern BOOL g_bOneSharpen;	//向下位机发送一次锐化
extern BOOL g_bOneR;		//向下位机发送一次R调节
extern BOOL g_bOneB;		//向下位机发送一次B调节
extern BOOL	g_bStop;		//特殊操作停止视频
extern BOOL g_bVideo;		//录像
extern BOOL g_bDeleteTemp;	//删除文件
extern BOOL g_bInputInfo;	//输入信息
extern BOOL g_bConnect;		//连接socket

extern ULONG g_uTmpPic;		//临时图片数

extern CString g_decDriver;	//u盘盘符
extern CString g_strSaveDir;//图片视频保存路径
extern CString g_strTmpDir;	//生成临时图片路径
extern CString g_ConfigPath;//配置文件路径
extern CString g_strIP;		//ip
extern CString g_strName;	//患者姓名
extern CString g_strID;		//患者ID
extern CString g_strPatientInfo;

extern CString strAvg;		//测试后删除
extern CString strPeek;		//测试后删除

extern queue<BYTE *> g_Video_list;	//存放帧缓存

extern map<unsigned int, double> g_Map_Sharpness;//锐化对应关系
extern queue<string> g_Operate_list;			 //操作记录列表

extern CWnd *g_pShowDlg;						 //操作窗口指针

class CComDataHandler
{
public:
	virtual void ProcessData( unsigned char* pBuf, int size) = 0;
};

class CRetryInfo  
{
public:
	short m_siFrame;
	short m_siRetryCount;
	CTime sendtime;
	BYTE sendbuf[64];
	int sendlength;
	bool operator == (const CRetryInfo &RetryInfo) const
	{
		if (this->m_siFrame == RetryInfo.m_siFrame)
			return true;
		else
			return false;
	}

protected:
private:
};

extern CString ReturnPath();

extern bool ReadConfig(void);

extern void SetConfig(CString keyname, double para);


