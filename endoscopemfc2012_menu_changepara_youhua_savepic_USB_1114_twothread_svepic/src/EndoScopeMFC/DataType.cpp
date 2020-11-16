#include "pch.h"
#include "DataType.h"

//unsigned char NOWENTER[ 14 ] = {0x0D, 0x82, 0x00, 0x31, 0xD5, 0xFD, 0xD4, 0xDA, 0xC2, 0xBC, 0xC8, 0xEB, 0xFF, 0xFF};	
ZOOMSTRUCT picZoom = {0, 0, 1280, 720, 150};

HANDLE g_hPic2VideoThread;
HANDLE g_hSaveVideoThread;
HANDLE g_hSaveDataThread;

int g_FPS = 0;//粗略统计帧率
int g_nSharpness = 0;			//清晰度
int g_nBrightness = 0;			//传感器亮度
int g_nContrast = 0;			//对比度
int g_nSaturation = 5;			//饱和度
int g_nExposure = 0;			//曝光
int g_nGain = 0;				//增益
int g_nWhitebalance=0;          //白平衡
int g_nLBrightness = 5;			//冷光源亮度
int g_nRed = 0;					//R调节
int g_nBlue = 0;				//B调节
int g_nMetering = 0;			//测光模式 0:平均 1:峰值
int g_nThresh = 255;//阈值
int g_nThreshNew = 255;//wm_20200919阈值
int g_nChangePixel = 3; //选择像素通道个数
int g_nChange255 = 0; //赋值255
int g_nShowShape = 0;			//显示 0:方角 1:圆角
int g_nPeek = 0;				//测光峰值
int g_ncout=0;//LJ731
int g_nAvg = 0;					//测光均值
int g_nNBI = 4;		 	        //NBI 0;1;2;3四个档位 4关闭NBI
int g_nVCOM = 0;				//视频串口
int g_nLCOM = 0;				//冷光源串口
int g_nCCOM=2;					//图像处理板串口
int g_nPort = 8887;				//port
double g_lamdar=1.0 ;
double g_lamdab=1.0;
double g_lamdag=1.0;
double g_fGainR = 0;			//白平衡R值
double g_fGainB = 0;			//白平衡B值
int g_iMenu = 0;
int g_thred;
BOOL g_bFICE = FALSE;			//FICE
BOOL g_bExposure = FALSE;		//自动曝光
BOOL g_bGain = FALSE;			//自动增益
BOOL g_bWhiteBalance = FALSE;	//自动白平衡

BOOL g_bIsLightOpen = FALSE;	//冷光源开关
BOOL g_nOneSave = FALSE;		//保存一次图片
BOOL g_bIsUpan = FALSE;			//u盘插入标记

BOOL g_bIsAutoLightOpen = TRUE;		//自动调节冷光源
BOOL g_bOneSharpen = FALSE;		//向下位机发送一次锐化
BOOL g_bOneR = FALSE;			//向下位机发送一次R调节
BOOL g_bOneB = FALSE;			//向下位机发送一次B调节
BOOL g_bStop = FALSE;			//特殊操作停止视频
BOOL g_bVideo = FALSE;			//录像

BOOL g_bDeleteTemp = FALSE;		//删除文件
BOOL g_bInputInfo = FALSE;		//输入信息
BOOL g_bConnect = FALSE;		//连接socket

ULONG g_uTmpPic = 0;			//临时图片数

CString g_decDriver = _T( "" );			//u盘盘符
//20180111
CString g_strSaveDir = _T("DATA\\");//图片视频保存路径
CString g_strTmpDir = _T("D:\\Temp\\");	//生成临时图片路径
CString g_ConfigPath = _T( "" );		//配置文件路径
CString g_strIP = _T("192.168.1.145");	//ip
CString g_strName = _T( "" );	//患者姓名
CString g_strID =_T( "" );		//患者ID
CString g_strPatientInfo=_T("");

CString strAvg = _T("");		//测试后删除
CString strPeek = _T("");		//测试后删除

queue<BYTE *> g_Video_list;	//存放帧缓存

map<unsigned int, double> g_Map_Sharpness;//锐化对应关系
queue<string> g_Operate_list;			 //操作记录列表

CWnd *g_pShowDlg = NULL;		//操作窗口指针

bool ReadConfig(void)
{
	CString strValue = _T(""),strValue1 = _T("");
	g_ConfigPath = ReturnPath() + _T("\\config.ini");

	GetPrivateProfileString(_T("PARAMETER"), _T("PATH"), _T("D:\\内窥镜数据\\"), g_strSaveDir.GetBuffer(512), 512, g_ConfigPath);
	g_strSaveDir.Trim();
	if (g_strSaveDir.Right(1) != "\\")
		g_strSaveDir.Append(_T("\\"));
	//g_nSharpness = GetPrivateProfileInt(_T("PARAMETER"), _T("SHARPNESS"), 1, g_ConfigPath);
	g_nBrightness = GetPrivateProfileInt(_T("PARAMETER"), _T("BRIGHTNESS"), 1, g_ConfigPath);
	g_nContrast = GetPrivateProfileInt(_T("PARAMETER"), _T("CONTRAST"), 1, g_ConfigPath);
	g_nSaturation = GetPrivateProfileInt(_T("PARAMETER"), _T("SATURATION"), 1, g_ConfigPath);
	g_nExposure = GetPrivateProfileInt(_T("PARAMETER"), _T("EXPOSURE"), 1, g_ConfigPath);
	g_nGain = GetPrivateProfileInt(_T("PARAMETER"), _T("GAIN"), 1, g_ConfigPath);
	g_nMetering = GetPrivateProfileInt(_T("PARAMETER"), _T("ADJBRIGHT"), 1, g_ConfigPath);
	g_bWhiteBalance = GetPrivateProfileInt(_T("PARAMETER"), _T("WHITEBALANCE"), 1, g_ConfigPath);
	g_nWhitebalance = GetPrivateProfileInt(_T("PARAMETER"), _T("WHITEBALANCEVAL"), 1, g_ConfigPath);
	g_nBlue = GetPrivateProfileInt(_T("PARAMETER"), _T("HUE_B"), 1, g_ConfigPath);
	g_nRed = GetPrivateProfileInt(_T("PARAMETER"), _T("HUE_R"), 1, g_ConfigPath);
	//g_bIsAutoLightOpen = GetPrivateProfileInt(_T("PARAMETER"), _T("AUTOLIGHT"), 1, g_ConfigPath);
	g_nShowShape = GetPrivateProfileInt(_T("PARAMETER"), _T("CONTUOR"), 0, g_ConfigPath); 
	g_bExposure = GetPrivateProfileInt(_T("PARAMETER"), _T("AUTOEXPOSURE"), 0, g_ConfigPath);
	g_thred=GetPrivateProfileInt(_T("PARAMETER"), _T("THRETH"), 0, g_ConfigPath);
	g_fGainR = GetPrivateProfileInt(_T("PARAMETER"), _T("GAIN_R"), 0, g_ConfigPath);
	g_fGainB = GetPrivateProfileInt(_T("PARAMETER"), _T("GAIN_B"), 0, g_ConfigPath);
	g_iMenu = GetPrivateProfileInt(_T("PARAMETER"), _T("MENU"), 3, g_ConfigPath);
	g_lamdar=GetPrivateProfileInt(_T("PARAMETER"), _T("LAMDAR"), 0, g_ConfigPath);
	g_lamdag=GetPrivateProfileInt(_T("PARAMETER"), _T("LAMDAG"), 0, g_ConfigPath);
	g_lamdab=GetPrivateProfileInt(_T("PARAMETER"), _T("LAMDAB"), 0, g_ConfigPath);
	g_nChangePixel = GetPrivateProfileInt(_T("PARAMETER"), _T("CHANGEPIXEL"), 3, g_ConfigPath);
	g_nThresh= GetPrivateProfileInt(_T("PARAMETER"), _T("THRETH2"), 255, g_ConfigPath);
	g_nChange255 = GetPrivateProfileInt(_T("PARAMETER"), _T("CHANGE255"), 0, g_ConfigPath);
	g_nThreshNew= GetPrivateProfileInt(_T("PARAMETER"), _T("THRETHNEW"), 255, g_ConfigPath);

	g_nVCOM = GetPrivateProfileInt(_T("COM"), _T("VCOMNUM"), 0, g_ConfigPath);
	g_nLCOM = GetPrivateProfileInt(_T("COM"), _T("LCOMNUM"), 0, g_ConfigPath);
	g_nCCOM=GetPrivateProfileInt(_T("COM"), _T("CCOMNUM"), 2, g_ConfigPath);

	g_bConnect = GetPrivateProfileInt(_T("SOCKET"), _T("ISCONNECT"), 0, g_ConfigPath);
	GetPrivateProfileString(_T("SOCKET"), _T("SERVERIP"), _T("192.168.1.248"), g_strIP.GetBuffer(32), 32, g_ConfigPath);
	g_nPort = GetPrivateProfileInt(_T("SOCKET"), _T("SERVERPORT"), 8887, g_ConfigPath);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void SetConfig(CString keyname, double para)//写配置文件
{
	CString temp;
	temp.Format(_T("%lf"), para);
	WritePrivateProfileString(_T("PARAMETER"), keyname, temp, g_ConfigPath);

 	CString cstrTmp;
 	cstrTmp.Format( _T("insert into RecData (操作, 操作值) Values('%s', '%lf')"), keyname, para);
	//USES_CONVERSION; 
 	string sql=(CT2A)cstrTmp.GetString(); 
	g_Operate_list.push( sql );

// 	bool ret = SQLiteHelper::GetInstance().InsertData(sql);  
// 	if (!ret)  
// 	{  
// 		string strTmp = SQLiteHelper::GetInstance().GetLastError();  
// 		g_Operate_list.pop();
// 		AfxMessageBox( _T("SaveError!") );
// 		return;  
// 	} 
}

CString ReturnPath()
{
	CString sPath;
	GetModuleFileName(NULL, sPath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	sPath.ReleaseBuffer();

	int nPos;
	nPos = sPath.ReverseFind('\\');
	sPath = sPath.Left(nPos);
	return sPath;
}