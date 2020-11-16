
// EndoScopeMFCDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "EndoScopeMFC.h"
#include "EndoScopeMFCDlg.h"
#include "afxdialogex.h"
#include "iostream"
#include "PLC.h"
#include "Dbt.h"
#include "Login.h"
#include "packQueue.h"
//#include "packQueueFrame.h"
#include "frameQueue.h"
#include "yuvQueue.h"
#include "PicQueue.h"

using namespace cv;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
# define SHOWTIME 1
# define SHOWFPS 2
# define WHITEBALANCE 3

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
extern std::list<BYTE*>g_lbufferinfo;
bool g_bprocthread=false;//控制图像处理线程是否运行
extern CCritSec g_dVideoLock;//录像缓存队列锁
static BOOL bSaveVideoStart=FALSE;//存储视频线程标志
CEvent g_videoevent(NULL,FALSE,FALSE,NULL);
CEvent g_resizeevent(NULL,FALSE,FALSE,NULL);
CEvent g_picevent(NULL,FALSE,FALSE,NULL);
CEvent g_showevent(NULL,FALSE,FALSE,NULL);
CEvent g_capevent(NULL,FALSE,FALSE,NULL);

static bool bshot=false;
static unsigned char picBuf[10][IMGSIZE] = {0};

char FirstDriveFromMask(ULONG unitmask)
{
	char i;
	for (i = 0; i < 26; ++i)
	{
		if (unitmask & 0x1)
			break;
		unitmask = unitmask >> 1;
	}
	return (i + 'A');
}

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
	msg1.Format("%s", "请检查外部存储设备是否正确插入");
	msg2.Format("%s", "若已插入则插拔一次");
	bHaveDlg = FALSE;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CAboutDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	bHaveDlg = FALSE;
	CDialog::OnClose();
}
BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetDlgItemText(IDC_STATIC1, msg1);
	SetDlgItemText(IDC_STATIC2, msg2);
	bHaveDlg = TRUE;
	return TRUE;
}
BOOL CAboutDlg::SetInfo(CString s1, CString s2)
{
	msg1.Format("%s", s1);
	msg2.Format("%s", s2);
	return TRUE;
}




// CEndoScopeMFCDlg 对话框



CEndoScopeMFCDlg::CEndoScopeMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ENDOSCOPEMFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CEndoScopeMFCDlg::~CEndoScopeMFCDlg()
{
	//if (m_pCCGrapgBuilder != NULL)
	//	delete m_pCCGrapgBuilder;
	//停止采图graph
	//关闭处理线程
	m_brecvvideo = false;
	m_bshowvideo = false;
	if (m_lamda)
	{
		delete[]m_lamda;
		m_lamda = NULL;
	}
	if (!m_src.empty())
	{
		m_src.release();
		//delete buf;
	}
	if(Picimg.empty())
	{
		Picimg.release();
	}
	if (!m_pTaskList.empty())
	{
		for (int i=0;i<m_pTaskList.size();++i)
		{
			BYTE* data = m_pTaskList.front();
			delete[]data;
			m_pTaskList.pop_front();
		}
		m_pTaskList.clear();
	}
	if (!m_pPicList.empty())
	{
		for (int i=0;i<m_pPicList.size();++i)
		{
			BYTE* data = m_pPicList.front();
			delete[]data;
			m_pPicList.pop_front();
		}
		m_pPicList.clear();
	}
	SAFE_DELETE(g_pPLC);
	g_bprocthread=false;
	CloseHandle(m_hthreadprocess);

}
void CEndoScopeMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEndoScopeMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUT_OK, &CEndoScopeMFCDlg::OnBnClickedButOk)
	ON_BN_CLICKED(IDC_BUT_RUN, &CEndoScopeMFCDlg::OnBnClickedButRun)
	ON_BN_CLICKED(IDC_BUT_PAUSE, &CEndoScopeMFCDlg::OnBnClickedButPause)
	ON_BN_CLICKED(IDC_BUT_STOP, &CEndoScopeMFCDlg::OnBnClickedButStop)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_SERIALPORT_MSG, OnCtlData)

	//	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CEndoScopeMFCDlg 消息处理程序

BOOL CEndoScopeMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_sdatapath = "DATA\\";
	if (!PathIsDirectory(m_sdatapath))
	{
		::CreateDirectory(m_sdatapath, NULL);//创建目录,已有的话不影响  
	}
	InitParameter();
	InitializePLC();
	InitializeFormBoxLayout();//设置窗口大小位置
	InitializecvWindow();//初始化opencv window
	SetPicWidowShape();//设置视频窗口形状
	IniWhilteLamda();//初始化白平衡
	SetInitValue();
	InitializeThread();//初始化线程
	/*官方采集*/
	//m_pCCGrapgBuilder = new MWCCGraphBuilder();

	//m_pCCGrapgBuilder->InitVMR9(m_hPicWindow);
	//m_pCCGrapgBuilder->EnumDevices();
	//m_pCCGrapgBuilder->CreateCCDecoder();
	/*回调采集*/
	m_eCurrent = EDS_Init;
	//CWnd *pwnd = GetDlgItem(IDC_STILL);
	//HWND hwnd = pwnd->GetSafeHwnd();
	HRESULT hr=m_dwcap.Init(0, m_hPicWindow);
	if (FAILED(hr)) {
		m_eCurrent = EDS_Stopped;
	}
	else
		m_eCurrent = EDS_Running;
	//GetDlgItem(IDC_STILL)->SetFocus();
	SetTimer(SHOWTIME, 1000,NULL);
	SetTimer(SHOWFPS, 1000, NULL);
	//g_bprocthread = true;
	//ResumeThread(m_hthreadprocess);
	//cout<<"main thread"<<std::this_thread::get_id() << endl;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CEndoScopeMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEndoScopeMFCDlg::OnPaint()
{
	//SetPicWidowShape();
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CEndoScopeMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//需要在初始化完成控件位置后调用
void CEndoScopeMFCDlg::InitializecvWindow()
{
	//设置opencv窗口	
	cv::namedWindow("view", WINDOW_NORMAL);
	//CRect rc;
	//GetDlgItem(IDC_STILL)->GetClientRect(rc);
	cv::resizeWindow("view", m_rPicRect.right - m_rPicRect.left-1, m_rPicRect.bottom - m_rPicRect.top-1);
	HWND hWnd = (HWND)cvGetWindowHandle("view");
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_STILL)->m_hWnd);
	//::SetParent(hWnd, m_hPicWindow);

	//::SetParent(hWnd, this->m_hWnd);//wm_1022
	//CWnd *pWnd = CWnd::FromHandle(hWnd);
	//pWnd->SetWindowPos(&CWnd::wndTopMost, m_nPicWindow_left, m_nPicWindow_top, m_nPicWindow_width, m_nPicWindow_height, SWP_SHOWWINDOW);
	//LONG_PTR Style = GetWindowLongPtr(m_hPicWindow,GWL_STYLE);   //获取xx2属性
	//Style = Style|WS_CLIPCHILDREN;         //xx2属性+WS_CLIPCHILDREN      即父窗口不再重绘子窗口覆盖部分
	//SetWindowLongPtr(m_hPicWindow, GWL_STYLE, Style);                       //重置xx2属性
	::ShowWindow(hParent, SW_HIDE);
	//::ShowWindow(hWnd, SW_HIDE);
	//::ShowWindow(m_hPicWindow,SW_SHOW);

}

LRESULT CEndoScopeMFCDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DEVICECHANGE:
		{
			PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
			switch(wParam)
			{
			case WM_DEVICECHANGE:
				break;
			case DBT_DEVICEARRIVAL:
				{
					if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)//逻辑卷
					{
						PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
						switch (lpdbv->dbcv_flags)
						{
						case 0://U盘
							{
								g_bIsUpan = TRUE;
								CString decDriver;
								decDriver = FirstDriveFromMask(lpdbv->dbcv_unitmask);
								g_decDriver = decDriver;
								GetDlgItem(IDC_STORAGE_INFO_STATIC)->SetWindowText(_T("已插入"));
								TRACE("检测到U盘:[%s]插入!\n", decDriver.GetBuffer(0));
								g_pPLC->OverSave();
							}
							break;
						}
					}
				}
				break;
			case DBT_DEVICEREMOVECOMPLETE://DBT_DEVICEREMOVECOMPLETE,设备卸载或者拔出
				{
					if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)//逻辑卷
					{
						PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
						switch (lpdbv->dbcv_flags)
						{
						case 0:                //U盘
							{
								g_bIsUpan = FALSE;
								CString decDriver;
								decDriver = FirstDriveFromMask(lpdbv->dbcv_unitmask);
								GetDlgItem(IDC_STORAGE_INFO_STATIC)->SetWindowText(_T("未插入"));
								TRACE("检测到U盘:[%s]拔出!\n", decDriver.GetBuffer(0));
								g_pPLC->NotLink();
								Sleep(100);
								g_pPLC->OverSave();//LJ
							}
							break;
						}
					}

				}
				break;
			}
		}
		break;

		//case WM_GRAPHNOTIFY:
		//HandleDshowEvent();
		//break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}
void CEndoScopeMFCDlg::SetInitValue()
{
	//1.相机亮度恢复
	int nBrightness = GetPrivateProfileInt(_T("FACTORY"), _T("BRIGHTNESS"), 1, g_ConfigPath);
	if((g_nBrightness<-10)||(g_nBrightness>10))
	{
		nBrightness=0;
	}
	g_pPLC->Brightness5();
	g_nBrightness = nBrightness;
	Sleep(20);
	//2.相机对比度恢复
	int nContrast = GetPrivateProfileInt(_T("FACTORY"), _T("CONTRAST"), 1, g_ConfigPath);
	if((nContrast<0)||(nContrast>10))
	{
		nContrast=0;
	}
	g_pPLC->Contrast(nContrast);
	g_nContrast = nContrast;
	Sleep(20);
	//3.相机饱和度恢复
	int nSaturation = GetPrivateProfileInt(_T("FACTORY"), _T("SATURATION"), 1, g_ConfigPath);
	if((nSaturation<0)||(nSaturation>10))
	{
		nSaturation=5;
	}
	g_pPLC->Saturation(nSaturation);
	g_nSaturation = nSaturation;
	Sleep(20);
	//13.色调R B
	int nRed = GetPrivateProfileInt(_T("FACTORY"), _T("HUE_R"), 1, g_ConfigPath);
	if((nRed<-9)||(nRed>9))
	{
		nRed = 0;
	}
	g_pPLC->R(nRed);
	int pos = m_strRB.Find('R');
	m_strRB.SetAt(++pos, '+');
	m_strRB.SetAt(++pos, '0');
	GetDlgItem(IDC_IMGCLR_INFO_STATIC)->SetWindowText(m_strRB);
	g_nRed = nRed;
	Sleep(20);

	int nBlue = GetPrivateProfileInt(_T("FACTORY"), _T("HUE_B"), 1, g_ConfigPath);
	if((nBlue<-9)||(nBlue>9))
	{
		nBlue = 0;
	}
	g_pPLC->B(nBlue);
	pos = m_strRB.Find('B');
	m_strRB.SetAt(++pos, '+');
	m_strRB.SetAt(++pos, '0');
	GetDlgItem(IDC_IMGCLR_INFO_STATIC)->SetWindowText(m_strRB);
	g_nBlue = nBlue;
	Sleep(20);
	//4.相机增益恢复
	int nGain = GetPrivateProfileInt(_T("FACTORY"), _T("GAIN"), 1, g_ConfigPath);
	if((nGain<0)||(nGain>10))
	{
		nGain=0;
	}
	g_pPLC->Gain(nGain);
	g_nGain = nGain;
	Sleep(20);
	//5.相机曝光恢复
	int nExposure = GetPrivateProfileInt(_T("FACTORY"), _T("EXPOSURE"), 1, g_ConfigPath);
	if((nExposure>0)||(nExposure<-10))
	{
		nExposure=0;
	}
	g_pPLC->Exposure2();
	g_nExposure = nExposure;
	//g_nSharpness = 6;
	Sleep(20);
	//6.白平衡设置
	//7.色调设置
	//8.清晰度设置
	//int nSharpness=GetPrivateProfileInt(_T("FACTORY"),_T("SHARPNESS"),1,g_ConfigPath);
	//if((nSharpness<0)||(nSharpness>15))
	//{
	//	nSharpness=4 ;
	//}
	//g_pPLC->Sharpness(nSharpness);
	//Sleep(5);
	//9.伽马设置
	//10.测光模式
	int nMetering = GetPrivateProfileInt(_T("FACTORY"), _T("ADJBRIGHT"), 1, g_ConfigPath);
	if(0 == nMetering)
	{
		g_nMetering = 0;
		g_pPLC->AverageMetering();
		GetDlgItem(IDC_TESTLIGHT_INFO_STATIC)->SetWindowText(_T("平均测光"));
	}
	else if(1 == nMetering)
	{
		g_nMetering = 1;
		g_pPLC->PeakMetering();
		GetDlgItem(IDC_TESTLIGHT_INFO_STATIC)->SetWindowText(_T("峰值测光"));
	}
	Sleep(20);
	//11.轮廓
	int nShowShape = GetPrivateProfileInt(_T("FACTORY"), _T("CONTUOR"), 0, g_ConfigPath);
	if(0 == nShowShape)
	{
		m_nPicWindowShape = 0;//设置成圆角

	}
	else if(1 == nShowShape)
	{
		m_nPicWindowShape = 1;//设置成方角
	}
	SetPicWidowShape();
	Sleep(20);
	//12.放大
	g_pPLC->Zoom1();
	GetDlgItem(IDC_ENLARGE_INFO_STATIC)->SetWindowText(_T("1.0倍"));
	SetZoom(1);
	m_nZoomIn = 1;
	Sleep(20);
}


/*函数作用：实例化PLC对象，包括光源下位机和图像下位机
*输入限制：在相机初始化之前调用
*/

void CEndoScopeMFCDlg::InitializePLC()
{

	g_pPLC->GetShowDlgPointer(this);
	g_pPLC = new CPLC();
	if (NULL == g_pPLC)
	{
		MessageBox(_T("PLC初始化失败!"));
		//exit(0);
	}
	CString tempPort;
	tempPort.Format(_T("COM%d"), g_nCCOM);
	if (!g_pPLC->ProcessCardInitialize(tempPort,CBR_115200))
	{
		AfxMessageBox( _T("图像处理串口初始化失败！") );
	}

	tempPort.Format(_T("COM%d"), g_nVCOM);
	if (!g_pPLC->VideoInitialize(tempPort, CBR_115200))
	{
		//AfxMessageBox( _T("图像处理串口初始化失败！") );
		//exit(0);
	}
#ifdef _LPort
	//tempPort.Format(_T("COM%d"), g_nLCOM);
	//if (!g_pPLC->LightInitialize(tempPort, CBR_115200))
	//{
	//	AfxMessageBox( _T("冷光源串口初始化失败！") );
		//exit(0);
	//}
#endif
}

void CEndoScopeMFCDlg::OnBnClickedButOk()
{
	/*官方采集*/
	//HRESULT hr = m_pCCGrapgBuilder->ConnectFilters(1);

}



void CEndoScopeMFCDlg::OnBnClickedButRun()
{
	if (m_eCurrent==EDS_Paused||m_eCurrent==EDS_Stopped)
	{
		m_dwcap.Run();
		m_eCurrent = EDS_Running;
	}
}

void CEndoScopeMFCDlg::InitParameter()
{
	m_badjwhite = false;
	m_bwhiteok = false;
	m_dZoom=1.0;
	roi_w = IMGWIDTH;
	roi_h = IMGHEIGHT;
	roi_c=(roi_w-IMGWIDTH)/2;
	roi_r=(roi_h-IMGHEIGHT)/2;
	m_src = Mat(Size(IMGWIDTH, IMGHEIGHT), CV_8UC3);
	m_lamda = new double[IMGSIZE];
	m_dst=Mat(Size(1536,864),CV_8UC3);
	Picimg = Mat(Size(IMGWIDTH, IMGHEIGHT), CV_8UC3);

	m_nPicWindow_width = 1536;
	m_nPicWindow_height = 864;
	m_nPicWindow_left = 300;//起始点
	m_nPicWindow_top = 100;//起始点
	m_hPicWindow = NULL;
	m_rPicRect = NULL;
	m_bwritevideo = false;//是否录屏
	m_bstartsave = false;
	m_bwritepicture = false;
	m_bstartpicture = false;
	m_bshowpause = false;
	m_nPic=0;//记录截屏数量，软件启动初始化为0
	m_bsetuplamda = true;//第一次激活更改
	m_bInputInfo = true;//第一次激活可以输入
	//键盘键值
	m_f3key=0;
	m_f4key=0;
	m_f5key=0;
	m_f6key=0;
	m_f11key=0;
	m_f2key=g_iMenu;
	m_f7key = 0;
	//获取屏幕分辨率
	m_nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	m_nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	m_nZoomIn=1;//初始1
	m_strRB="R+0:B+0";
	//这块做什么用的
	m_hDeskWindow = GetDesktopWindow()->GetSafeHwnd();
	m_hDesktopDC = ::GetDC(m_hDeskWindow);
	m_hBmpFileDC = CreateCompatibleDC(m_hDesktopDC);
	//GetDlgItem(IDC_STILL, &m_hPicWindow);//获取显示窗口句柄
	//m_hPicWindow = GetDlgItem(IDC_STILL)->GetSafeHwnd();//获取显示窗口句柄
	//GetDlgItem(IDC_STILL)->GetClientRect(m_rPicRect);
	m_hBitmap = CreateCompatibleBitmap(m_hDesktopDC, m_nScreenWidth, m_nScreenHeight);
	SelectObject(m_hBmpFileDC, m_hBitmap);
	//if(1 == g_nShowShape)
	//{
	//	m_nPicWindowShape = 0;//设置成圆角

	//}
	//else if(0 == g_nShowShape)
	//{
	//	m_nPicWindowShape = 1;//设置成方角
	//}

	SetEvent(g_videoevent);
	SetEvent(g_resizeevent);
	SetEvent(g_capevent);
}

void CEndoScopeMFCDlg::OnBnClickedButPause()
{
	/*OAFilterState t_State;
	HRESULT hr = m_pCCGrapgBuilder->m_pMediaControl->GetState(10, &t_State);
	if (hr == S_OK || hr == VFW_S_CANT_CUE) {
		hr = E_FAIL;
		switch (t_State)
		{
		case State_Running:
			hr = m_pCCGrapgBuilder->m_pMediaControl->Pause();
			break;
		case State_Paused:
			hr = S_OK;
			break;
		case State_Stopped:
		default:
			break;
		}
	}*/
	/*回调方式*/
	if (m_eCurrent==EDS_Running)
	{
		m_dwcap.Pause();
		m_eCurrent = EDS_Paused;
	}
}

void CEndoScopeMFCDlg::OnBnClickedButStop()
{
	if (m_eCurrent==EDS_Running||m_eCurrent==EDS_Paused)
	{
		m_dwcap.Stop();
		m_eCurrent = EDS_Stopped;
	}
}

UINT CEndoScopeMFCDlg::SavePicThreadProc(LPVOID lpParameter)
{
#if 1
	CEndoScopeMFCDlg* pdlg = (CEndoScopeMFCDlg*)lpParameter;
	Mat img = Mat(Size(IMGWIDTH, IMGHEIGHT), CV_8UC3);
	Mat gray,laplac,meanstdev;
	Mat kernel1 = (Mat_<float>(2, 2) << 2,0,0,-2);
	Mat kernel2 = (Mat_<float>(2, 2) << 0,-2,2,0);
	Mat filter1,filter2,filter;
	Scalar     mean;
	Scalar     stddev;
	int64 val1,val2,val3,val4,val5,val6,val7,val8,val9,val10;
	int64 max = 0;
	int64 val = 0;
	int ret = 0;
	int len = 0;
	int num = 0;

	while (pdlg->m_bstartpicture)
	{
		ret = ReadPicQueue(img.data, &len);
		if (len <= 0 || ret < 0){
			Sleep(1);
			//pdlg->SetDlgItemInt(IDC_AGE_EDIT,1);
			continue;
		}
		if (!img.empty())
		{
		//	pdlg->SetDlgItemInt(IDC_AGE_EDIT,1);
			memset(picBuf[num],0,IMGSIZE);
			memcpy(picBuf[num],img.data,IMGSIZE);
	//		WriteYUVQueue(img.data,IMGSIZE);
	//		pdlg->SetDlgItemInt(IDC_AGE_EDIT,2);
#if 1
			//计算方差
			cvtColor(img, gray, COLOR_BGR2GRAY);
			//Laplacian(gray, laplac, CV_16U,3);
			//convertScaleAbs(laplac, meanstdev);

			//cvtColor(src11, gray, COLOR_BGR2GRAY);
			filter2D(gray, filter1, CV_8UC3, kernel1);
			filter2D(gray, filter2, CV_8UC3, kernel2);
			filter = filter1+filter2;
			meanStdDev(filter,mean,stddev);
			val = stddev.val[0]*1000000;
			//meanStdDev(meanstdev,mean,stddev);

			//val = cv::mean(meanstdev)[0]*10000000;

			if(0 == num)
			{
				val1 = val;
			}
			else if(1 == num)
			{
				val2 = val;
			}
			else if(2 == num)
			{
				val3 = val;
			}
			else if(3 == num)
			{
				val4 = val;
			}
			else if(4 == num)
			{
				val5 = val;
			}
			else if(5 == num)
			{
				val6 = val;
			}
			else if(6 == num)
			{
				val7 = val;
			}
			else if(7 == num)
			{
				val8 = val;
			}
			else if(8 == num)
			{
				val9 = val;
			}
			else if(9 == num)
			{
				val10 = val;
			}
			if(val>max)
			{
				max = val;
			}
#endif
			num++;
			string t_imgpath = pdlg->GetSaveImgPath(num);
			imwrite(t_imgpath, img);
			//if(9 == num)
			//{
			//	pdlg->m_bshowpause = true;
			//}
			if(10==num)
			{
			//	pdlg->SetDlgItemInt(IDC_AGE_EDIT,2);
			//	t_imgpath = pdlg->GetSaveImgPath(11);
				//暂停播放
				if (pdlg->m_eCurrent==EDS_Running)
				{
					pdlg->m_dwcap.Pause();
					pdlg->m_eCurrent = EDS_Paused;
				}
				if(max == val1)
				{
					memset(pdlg->Picimg.data,0,IMGSIZE);
					memcpy(pdlg->Picimg.data,&picBuf[0][0],IMGSIZE);
					imshow("view",pdlg-> Picimg);
					//pdlg->SetDlgItemInt(IDC_DOCTOR_EDIT,1);
				//	imwrite(t_imgpath, pdlg->Picimg);
				}
				else if(max == val2)
				{
					memset(pdlg->Picimg.data,0,IMGSIZE);
					memcpy(pdlg->Picimg.data,&picBuf[1][0],IMGSIZE);
					imshow("view", pdlg->Picimg);
					//pdlg->SetDlgItemInt(IDC_DOCTOR_EDIT,2);
				//	imwrite(t_imgpath, pdlg->Picimg);
				}
				else if(max == val3)
				{
					memset(pdlg->Picimg.data,0,IMGSIZE);
					memcpy(pdlg->Picimg.data,&picBuf[2][0],IMGSIZE);
					imshow("view", pdlg->Picimg);
					//pdlg->SetDlgItemInt(IDC_DOCTOR_EDIT,3);
				//	imwrite(t_imgpath, pdlg->Picimg);
				}
				else if(max == val4)
				{
					memset(pdlg->Picimg.data,0,IMGSIZE);
					memcpy(pdlg->Picimg.data,&picBuf[3][0],IMGSIZE);
					imshow("view", pdlg->Picimg);
					//pdlg->SetDlgItemInt(IDC_DOCTOR_EDIT,4);
				//	imwrite(t_imgpath, pdlg->Picimg);
				}
				else if(max == val5)
				{
					memset(pdlg->Picimg.data,0,IMGSIZE);
					memcpy(pdlg->Picimg.data,&picBuf[4][0],IMGSIZE);
					imshow("view", pdlg->Picimg);
					//pdlg->SetDlgItemInt(IDC_DOCTOR_EDIT,5);
				//	imwrite(t_imgpath, pdlg->Picimg);
				}
				else if(max == val6)
				{
					memset(pdlg->Picimg.data,0,IMGSIZE);
					memcpy(pdlg->Picimg.data,&picBuf[5][0],IMGSIZE);
					imshow("view", pdlg->Picimg);
				//	pdlg->SetDlgItemInt(IDC_DOCTOR_EDIT,6);
				//	imwrite(t_imgpath, pdlg->Picimg);
				}
				else if(max == val7)
				{
					memset(pdlg->Picimg.data,0,IMGSIZE);
					memcpy(pdlg->Picimg.data,&picBuf[6][0],IMGSIZE);
					imshow("view", pdlg->Picimg);
					//pdlg->SetDlgItemInt(IDC_DOCTOR_EDIT,7);
				//	imwrite(t_imgpath, pdlg->Picimg);
				}
				else if(max == val8)
				{
					memset(pdlg->Picimg.data,0,IMGSIZE);
					memcpy(pdlg->Picimg.data,&picBuf[7][0],IMGSIZE);
					imshow("view", pdlg->Picimg);
					//pdlg->SetDlgItemInt(IDC_DOCTOR_EDIT,8);
				//	imwrite(t_imgpath, pdlg->Picimg);
				}
				else if(max == val9)
				{
					memset(pdlg->Picimg.data,0,IMGSIZE);
					memcpy(pdlg->Picimg.data,&picBuf[8][0],IMGSIZE);
					imshow("view", pdlg->Picimg);
					//pdlg->SetDlgItemInt(IDC_DOCTOR_EDIT,9);
				//	imwrite(t_imgpath, pdlg->Picimg);
				}
				else if(max == val10)
				{
					memset(pdlg->Picimg.data,0,IMGSIZE);
					memcpy(pdlg->Picimg.data,&picBuf[9][0],IMGSIZE);
					imshow("view", pdlg->Picimg);
					//pdlg->SetDlgItemInt(IDC_DOCTOR_EDIT,10);
				//	imwrite(t_imgpath, pdlg->Picimg);
				}
			//	memset(picBuf[11],0,IMGSIZE+1);
				pdlg->m_bstartpicture = false;
				//pdlg->m_bshowpause = false;
				//bshot = true;
			//	pdlg->SetDlgItemInt(IDC_AGE_EDIT,3);
			}
		}
		Sleep(1);	
	}
#endif
	//pdlg->SetDlgItemInt(IDC_AGE_EDIT,4);
	/*pdlg->m_bstartpicture = false;*/
	img.release();
	bshot = true;
	return 0;
}

void CEndoScopeMFCDlg::ShotPicWindow()
{
	if (!bshot)
	{
		if(!m_bstartpicture)
		{
			m_bstartpicture = true;
		}
		else
		{
			return;
		}
		m_bwritepicture = true;
		m_bshowpause = true;
		pThreadPic = AfxBeginThread(SavePicThreadProc, this);		
		m_nPic++;
		CString strPicNum;
		strPicNum.Format(_T("%d"), m_nPic);
		GetDlgItem(IDC_IMAGENUM_INFO_STATIC)->SetWindowText(strPicNum);
	}
	else
	{
		m_bshowpause = false;
		//m_dwcap.StopWritePic();
		//m_bwritepicture = false;
		if (m_eCurrent == EDS_Paused)
		{
			m_dwcap.Run();
			m_eCurrent = EDS_Running;
		}
		bshot = false;
	}
	//GetDlgItem(IDC_STILL)->SetFocus();//这个很重要，要将当前 窗口激活状态
	this->SetFocus();
	//bshot=!bshot;
}
string CEndoScopeMFCDlg::GetSaveImgPath(int flag)
{
	if(!g_bIsUpan)
	{
		m_sdatapath = "DATA\\";
		if (!PathIsDirectory(m_sdatapath))
		{
			::CreateDirectory(m_sdatapath, NULL);//创建目录,已有的话不影响  
		}
	}
	CString t_path = m_sdatapath;
	CString t_sid;
	GetDlgItem(IDC_NAME_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ID_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_SEX_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_AGE_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DOCTOR_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ID_EDIT)->GetWindowText(t_sid);
	m_bInputInfo=true;
	g_pPLC->OverEnter();
	t_sid.TrimLeft(_T(" "));//删除前面的空格
	t_sid.TrimRight(_T(" "));//删除后面的空格
	if (t_sid.IsEmpty())
	{
		t_sid = _T("temp");
	}
	t_path.Append(t_sid);
	if (!PathIsDirectory(t_path))
	{
		::CreateDirectory(t_path, NULL);//创建目录,已有的话不影响  
	}
	t_path.Append(_T("\\"));
	COleDateTime tm = COleDateTime::GetCurrentTime();
	CString strFileName;
	t_path.Append(_T("\\"));
	CString fileend="";
	//if (flag==1)
	//{
	//	fileend=L".tiff";
	//}
	//else if (flag==2)
	//{
	//	fileend=L".mp4";
	//}
	//strFileName.Format(_T("%s%s%02d%s"), t_path, tm.Format(_T("%Y_%m_%d_%H_%M_%S")), 1, fileend);
	if(flag==0)
	{
		fileend=L".mp4";
		strFileName.Format(_T("%s%s%02d%s"), t_path, tm.Format(_T("%Y_%m_%d_%H_%M_%S")), 1, fileend);
	}
	else /*if(flag==1)*/
	{
		fileend=L".tiff";
		strFileName.Format(_T("%s%s_%d%s"), t_path, tm.Format(_T("%Y_%m_%d_%H_%M_%S")), flag, fileend);
	}
	return CT2A(strFileName);
}
void CEndoScopeMFCDlg::SaveCaptureImage(string filepath)
{
	char*    screenCaptureData = GetScreenDataGDI();
	m_SaveImage.create(m_nScreenHeight, m_nScreenWidth, CV_8UC4);
	memcpy(m_SaveImage.data, screenCaptureData, m_nScreenWidth*m_nScreenHeight * 4);
	Mat t_img = Mat(m_nScreenHeight, m_nScreenWidth, CV_8UC3);
	cvtColor(m_SaveImage, t_img, COLOR_BGRA2BGR);
	imwrite(filepath, t_img);
	delete[]screenCaptureData;
}

/*
*功能：初始化界面及控件的位置和大小
*/
void CEndoScopeMFCDlg::InitializeFormBoxLayout()
{
	//ModifyStyle(WS_CAPTION,0,0);  // 如果只是要普通的全屏，不想去掉标题栏，就不用第一个语句
	//SendMessage(WM_SYSCOMMAND,SC_MAXIMIZE,0);
	RECT rect;
	CFont myFont20;
	rect.top = 0;
	rect.bottom = 1080;
	rect.left = 0;
	rect.right = 1920;
	SetWindowPos(this, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER);//设置主对话框大小和位置
	m_hPicWindow = GetDlgItem(IDC_STILL)->GetSafeHwnd();//获取显示窗口句柄
	//SetWindowPos(&CWnd::wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	GetDlgItem(IDC_STILL)->SetWindowPos(NULL, m_nPicWindow_left, m_nPicWindow_top, m_nPicWindow_width, m_nPicWindow_height, SWP_NOZORDER);
	DWORD style = GetClassLong(m_hPicWindow, GCL_STYLE);
	SetClassLong(m_hPicWindow, GCL_STYLE, style & ~CS_PARENTDC);
	GetDlgItem(IDC_STILL)->GetClientRect(m_rPicRect);

	myFont20.CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("宋体"));
	GetDlgItem(IDC_PEOPLEMSG_STATIC)->SetWindowPos(NULL, 50, 120, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_ID_STATIC)->SetWindowPos(NULL, 50, 50 + 100, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_NAME_STATIC)->SetWindowPos(NULL, 50, 80 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_SEX_STATIC)->SetWindowPos(NULL, 50, 110 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_AGE_STATIC)->SetWindowPos(NULL, 50, 140 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_DOCTOR_STATIC)->SetWindowPos(NULL, 50, 170 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_IMGMSG_STATIC)->SetWindowPos(NULL, 50, 220 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_TESTLIGHT_STATIC)->SetWindowPos(NULL, 50, 250 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_IMGCLR_STATIC)->SetWindowPos(NULL, 50, 280 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_ENLARGE_STATIC)->SetWindowPos(NULL, 50, 310 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_FORM_STATIC)->SetWindowPos(NULL, 50, 340 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_ENDOSCOPEMSG_STATIC)->SetWindowPos(NULL, 50, 390 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_ENDOSCOPEID_STATIC)->SetWindowPos(NULL, 50, 420 + 100 + 5, 100, 20, SWP_NOZORDER);
	GetDlgItem(IDC_DISPOSABLE_STATIC)->SetWindowPos(NULL, 50, 450 + 100 + 5, 150, 20, SWP_NOZORDER);
	GetDlgItem(IDC_STOREMSG_STATIC)->SetWindowPos(NULL, 50, 500 + 100 + 5, 200, 20, SWP_NOZORDER);
	GetDlgItem(IDC_STORAGE_STATIC)->SetWindowPos(NULL, 50, 530 + 100 + 5, 150, 20, SWP_NOZORDER);
	GetDlgItem(IDC_IMAGENUM_STATIC)->SetWindowPos(NULL, 50, 560 + 100 + 5, 150, 20, SWP_NOZORDER);
	GetDlgItem(IDC_RECORD_STATIC)->SetWindowPos(NULL, 50, 590 + 100 + 5, 150, 20, SWP_NOZORDER);
	GetDlgItem(IDC_TIME_STATIC)->SetWindowPos(NULL, 50, 1000, 220, 20, SWP_NOZORDER);

	GetDlgItem(IDC_BUT_OK)->SetWindowPos(NULL, 50, 760, 220, 20, SWP_NOZORDER);
	GetDlgItem(IDC_BUT_RUN)->SetWindowPos(NULL, 50, 840, 220, 20, SWP_NOZORDER);
	GetDlgItem(IDC_BUT_PAUSE)->SetWindowPos(NULL, 50, 880, 220, 20, SWP_NOZORDER);
	GetDlgItem(IDC_BUT_STOP)->SetWindowPos(NULL, 50, 950, 220, 20, SWP_NOZORDER);

	GetDlgItem(IDC_ID_EDIT)->SetWindowPos(NULL, 160, 50 + 100, 125, 30, SWP_NOZORDER);

	GetDlgItem(IDC_NAME_EDIT)->SetWindowPos(NULL, 160, 80 + 100 + 5, 125, 30, SWP_NOZORDER);
	GetDlgItem(IDC_SEX_EDIT)->SetWindowPos(NULL, 160, 110 + 100 + 5, 125, 30, SWP_NOZORDER);
	GetDlgItem(IDC_AGE_EDIT)->SetWindowPos(NULL, 160, 140 + 100 + 5, 125, 30, SWP_NOZORDER);
	GetDlgItem(IDC_DOCTOR_EDIT)->SetWindowPos(NULL, 160, 170 + 100 + 5, 125, 30, SWP_NOZORDER);
	GetDlgItem(IDC_TESTLIGHT_INFO_STATIC)->SetWindowPos(NULL, 160, 250 + 100 + 5, 125, 20, SWP_NOZORDER);
	GetDlgItem(IDC_IMGCLR_INFO_STATIC)->SetWindowPos(NULL, 160, 280 + 100 + 5, 125, 20, SWP_NOZORDER);
	GetDlgItem(IDC_ENLARGE_INFO_STATIC)->SetWindowPos(NULL, 160, 310 + 100 + 5, 125, 20, SWP_NOZORDER);
	GetDlgItem(IDC_FORM_INFO_STATIC)->SetWindowPos(NULL, 160, 340 + 100 + 5, 125, 20, SWP_NOZORDER);
	GetDlgItem(IDC_ENDOSCOPEID_INFO_STATIC)->SetWindowPos(NULL, 200, 420 + 100 + 5, 85, 20, SWP_NOZORDER);
	GetDlgItem(IDC_DISPOSABLE_INFO_STATIC)->SetWindowPos(NULL, 200, 450 + 100 + 5, 85, 20, SWP_NOZORDER);
	GetDlgItem(IDC_STORAGE_INFO_STATIC)->SetWindowPos(NULL, 200, 530 + 100 + 5, 85, 20, SWP_NOZORDER);
	GetDlgItem(IDC_IMAGENUM_INFO_STATIC)->SetWindowPos(NULL, 200, 560 + 100 + 5, 85, 20, SWP_NOZORDER);
	GetDlgItem(IDC_RECORD_INFO_STATIC)->SetWindowPos(NULL, 200, 590 + 100 + 5, 85, 20, SWP_NOZORDER);
	GetDlgItem(IDC_PEOPLEMSG_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_NAME_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_ID_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_SEX_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_AGE_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_DOCTOR_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_IMGMSG_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_TESTLIGHT_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_IMGCLR_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_ENLARGE_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_FORM_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_ENDOSCOPEMSG_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_ENDOSCOPEID_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_DISPOSABLE_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_STOREMSG_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_STORAGE_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_IMAGENUM_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_RECORD_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_TIME_STATIC)->SetFont(&myFont20);
	GetDlgItem(IDC_NAME_EDIT)->SetFont(&myFont20);
	GetDlgItem(IDC_ID_EDIT)->SetFont(&myFont20);
	GetDlgItem(IDC_SEX_EDIT)->SetFont(&myFont20);
	GetDlgItem(IDC_AGE_EDIT)->SetFont(&myFont20);
	GetDlgItem(IDC_DOCTOR_EDIT)->SetFont(&myFont20);
	GetDlgItem(IDC_NAME_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ID_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_SEX_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_AGE_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DOCTOR_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECORD_INFO_STATIC)->SetWindowText(_T("准备就绪"));
	GetDlgItem(IDC_STORAGE_INFO_STATIC)->SetWindowText(_T("未插入"));
	//设置内镜信息
	GetDlgItem(IDC_DISPOSABLE_INFO_STATIC)->SetWindowText(_T("一次性"));
	GetDlgItem(IDC_ENDOSCOPEID_INFO_STATIC)->SetWindowText(_T("310D"));
	myFont20.CreateFont(40, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("宋体"));
	
	GetDlgItem(IDC_STATIC_INFOBOX)->SetWindowPos(NULL, 1000, 50, 200, 30, SWP_NOZORDER);
	GetDlgItem(IDC_STATIC_INFOBOX)->SetFont(&myFont20);
	//GetDlgItem(IDC_STATIC_INFOBOX)->ShowWindow(SW_HIDE);
	

}
/*
*功能：更改对话框和控件的背景颜色为黑色
*/
//一次只创建一个即可，在函数内部会创建多个导致资源泄漏
HBRUSH b = CreateSolidBrush(RGB(0, 0, 0));
HBRUSH CEndoScopeMFCDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);


	if (nCtlColor == CTLCOLOR_EDIT)   //更改编辑框
	{
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkColor(RGB(0, 0, 0));
		return b;
	}
	else if (nCtlColor == CTLCOLOR_STATIC)  //更改静态文本
	{

		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkColor(RGB(0, 0, 0));
		return b;
	}
	else if (nCtlColor == CTLCOLOR_DLG)   //更改对话框背景色
	{
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkColor(RGB(0, 0, 0));
		return b;
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
/*
*功能：设置视频显示窗口形状
*/
void CEndoScopeMFCDlg::SetPicWidowShape()
{
	//CRect rc;
	//GetDlgItem(IDC_STILL)->GetClientRect(rc);
	if (0 == m_nPicWindowShape)
	{
		m_nPicWindowShape = 1;
		GetDlgItem(IDC_FORM_INFO_STATIC)->SetWindowText(_T("圆角"));
		g_pPLC->SetCircular();
		CRgn roundrgn;
		//创建一个带圆角的矩形，左上角x,y，右下角x,y，圆角高宽
		roundrgn.CreateRoundRectRgn(2, 2, m_rPicRect.Width()-1, m_rPicRect.Height()-1, 150, 150);
		//InvalidateRgn(&roundrgn,TRUE);
		//::InvalidateRgn(m_hPicWindow,roundrgn,TRUE);
		::SetWindowRgn(m_hPicWindow,roundrgn, TRUE);
		//::InvalidateRgn(m_hPicWindow,roundrgn,TRUE);
		roundrgn.DeleteObject();
		//LONG_PTR Style = ::GetWindowLongPtr(m_hPicWindow,GWL_STYLE);   //获取xx2属性
		//Style = Style|WS_CLIPCHILDREN;         //xx2属性+WS_CLIPCHILDREN      即父窗口不再重绘子窗口覆盖部分
		//::SetWindowLongPtr(m_hPicWindow, GWL_STYLE, Style);
	}
	else
	{
		//m_nPicWindowShape = 0;
		//CRgn rox;
		//rox.CreateEllipticRgn(2,2,800,800); //画出圆形
		//::SetWindowRgn(m_hPicWindow,rox, TRUE);
		//DeleteObject(rox);
		//::ShowWindow(m_hPicWindow, SW_HIDE);
		m_nPicWindowShape = 0;
		GetDlgItem(IDC_FORM_INFO_STATIC)->SetWindowText(_T("方角"));
		g_pPLC->SetSquare();

		CRgn m_rectrgn;
		CPoint ptVertex[8];
		int diffrect = 50;
		ptVertex[0].x = 2;
		ptVertex[0].y = diffrect;
		ptVertex[1].x = diffrect;
		ptVertex[1].y = 2;
		ptVertex[2].x = m_rPicRect.Width() - diffrect;
		ptVertex[2].y = 2;
		ptVertex[3].x = m_rPicRect.Width() - 2;
		ptVertex[3].y = diffrect;
		ptVertex[4].x = m_rPicRect.Width() - 2;
		ptVertex[4].y = m_rPicRect.Height() - diffrect;
		ptVertex[5].x = m_rPicRect.Width() - diffrect;
		ptVertex[5].y = m_rPicRect.Height() - 2;
		ptVertex[6].x = diffrect;
		ptVertex[6].y = m_rPicRect.Height() - 2;
		ptVertex[7].x = 2;
		ptVertex[7].y = m_rPicRect.Height() - diffrect;

		//wm_1021
		m_rectrgn.CreatePolygonRgn(ptVertex, 8, WINDING);//WINDING ALTERNATE
		//InvalidateRgn(&m_rectrgn,TRUE);
		//::InvalidateRgn(m_hPicWindow,m_rectrgn,TRUE);
		::SetWindowRgn(m_hPicWindow,m_rectrgn, TRUE);
		//::InvalidateRgn(m_hPicWindow,m_rectrgn,TRUE);
		m_rectrgn.DeleteObject();
		//LONG_PTR Style = ::GetWindowLongPtr(m_hPicWindow,GWL_STYLE);   //获取xx2属性
		//Style = Style|WS_CLIPCHILDREN;         //xx2属性+WS_CLIPCHILDREN      即父窗口不再重绘子窗口覆盖部分
		//::SetWindowLongPtr(m_hPicWindow, GWL_STYLE, Style);
	}
}

//void CEndoScopeMFCDlg::OnSize(UINT nType, int cx, int cy)
//{
//	CDialogEx::OnSize(nType, cx, cy);
//	if (nType == 1)
//		return;
//	CWnd *t_hwndStatic = GetDlgItem(IDC_STATIC);
//	if (t_hwndStatic) {
//		ChangeSize(t_hwndStatic, cx, cy);
//		HWND hwnd;
//		GetDlgItem(IDC_STATIC, &hwnd);
//		//CDC *cdc = t_hwndStatic->GetDC();
//		//HDC hdc = cdc->GetSafeHdc();
//		//m_pCCGrapgBuilder->m_pVMR9WindowlessControl->RepaintVideo(hwnd, hdc);
//		RECT rec;
//		t_hwndStatic->GetClientRect(&rec);
//		m_pCCGrapgBuilder->m_pVMR9WindowlessControl->SetVideoPosition(NULL, &rec);
//	}
//	GetClientRect(&m_Rect);
//}

void CEndoScopeMFCDlg::ChangeSize(CWnd *hwnd, int cx, int cy)
{
	//if (hwnd == NULL)
	//	return;
	//CRect t_Rect;
	//hwnd->GetWindowRect(&t_Rect);
	//ScreenToClient(&t_Rect);

	//t_Rect.left = t_Rect.left*cx / m_Rect.Width();
	//t_Rect.right = t_Rect.right*cx / m_Rect.Width();
	//t_Rect.top = t_Rect.top*cy / m_Rect.Height();
	//t_Rect.bottom = t_Rect.bottom*cy / m_Rect.Height();
	//hwnd->MoveWindow(t_Rect);
}

void CEndoScopeMFCDlg::CaptureScreen()
{

}

//视频缩放
void CEndoScopeMFCDlg::VideoZoom()
{

	switch (m_nZoomIn)
	{
	case 0:
		//原始1倍
		g_pPLC->Zoom1();
		GetDlgItem(IDC_ENLARGE_INFO_STATIC)->SetWindowText(_T("1.0倍"));
		SetZoom(1);
		m_nZoomIn++;
		break;
	case 1:
		//放大1.2倍
		g_pPLC->Zoom2();
		GetDlgItem(IDC_ENLARGE_INFO_STATIC)->SetWindowText(_T("1.2倍"));
		SetZoom(1.2);
		m_nZoomIn++;
		break;
	case 2:
		//放大1.5倍
		g_pPLC->Zoom3();
		GetDlgItem(IDC_ENLARGE_INFO_STATIC)->SetWindowText(_T("1.5倍"));
		SetZoom(1.5);
		m_nZoomIn++;
		break;
	case 3:
		//放大2倍    
		g_pPLC->Zoom4();
		GetDlgItem(IDC_ENLARGE_INFO_STATIC)->SetWindowText(_T("2.0倍"));
		SetZoom(2);
		m_nZoomIn=0;
		break;
	default:
		break;
	}
}

BOOL CEndoScopeMFCDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		m_dwcap.Stop();
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F3)
	{
		if (m_f3key==0)
		{
			VideoZoom();
			m_f3key=1;
		}
	}
	else if(pMsg->message == WM_KEYUP && pMsg->wParam == VK_F3)
	{
		if (m_f3key==1)
		{
			m_f3key = 0;
		}
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F4)
	{
		//录入信息
		if (m_f4key==0)
		{
			EnterPatientInfo();
			m_f4key = 1;
		}
		
	}
	else if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_F4)
	{
		if (m_f4key==1)
		{
			m_f4key = 0;
		}
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F5)
	{
		//冻结图像
		if (m_f5key==0)
		{
			ShotPicWindow();
			m_f5key = 1;
		}
		
	}
	else if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_F5)
	{
		if (m_f5key==1)
		{
			m_f5key = 0;
		}
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F6)
	{
		if (m_f6key==0)
		{
			SetPicWidowShape();
			m_f6key = 1;
		}
		
	}
	else if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_F6)
	{
		if (m_f6key==1)
		{
			m_f6key = 0;
		}
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F7)
	{
		
		//UpdateWhiteBalance();
		if(0 == m_f7key)
		{
			//平均测光
			g_nMetering = 0;
			g_pPLC->AverageMetering();
			GetDlgItem(IDC_TESTLIGHT_INFO_STATIC)->SetWindowText(_T("平均测光"));
			m_f7key = 1;
		}
		else if(1 == m_f7key)
		{
			//峰值测光
			g_nMetering = 1;
			g_pPLC->PeakMetering();
			GetDlgItem(IDC_TESTLIGHT_INFO_STATIC)->SetWindowText(_T("峰值测光"));
			m_f7key = 0;
		}

	}
	else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F8)
	{
		MoveDatatoUSBDevice();
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F11)
	{
		//录屏
		if (m_f11key==0)
		{
			SaveVideo();
			
			m_f11key = 1;
		}
	}
	else if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_F11)
	{
		if (m_f11key==1)
		{
			m_f11key = 0;
		}
	}
	else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F2)
	{
		if(m_f2key==0)
		{
			MenuStart();
			m_f2key = 1;
		}
		else if(m_f2key==1)
		{
			MenuStop();
			m_f2key = 0;

		}

	}
	else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_UP)
	{
		if(1 == m_f2key)
		{
			MenuUp();
		}
		else if(3 == m_f2key)
		{
			if (g_nBrightness < 10)
			{
				BrightnessAdd();
			}
		}
		

	}
	else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DOWN)
	{
		if(1 == m_f2key)
		{
			MenuDown();
		}
		else if(3 == m_f2key)
		{
			if (g_nBrightness > -10)
			{
				BrightnessSub();
			}
		}
	}
	else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_LEFT)
	{
		if(1 == m_f2key)
		{
			MenuLeft();
		}
	}
	else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RIGHT)
	{
		if(1 == m_f2key)
		{
			MenuRight();
		}

	}
	else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F12)
	{
		if(1 == m_f2key)
		{
			MenuEnter();
		}
		else if(3 == m_f2key)
		{
			RestoreFactorySetting();
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

//显示当前时间
void CEndoScopeMFCDlg::ShowCurrentDateTime()
{
	CString t_sdt;
	CTime t_tdt;
	t_tdt = CTime::GetCurrentTime();
	t_sdt.Format(_T("%.4d年%.2d月%.2d日%.2d时%.2d分"), t_tdt.GetYear(), t_tdt.GetMonth(), t_tdt.GetDay(), t_tdt.GetHour(), t_tdt.GetMinute()); //获取年份
	GetDlgItem(IDC_TIME_STATIC)->SetWindowText(t_sdt);
}
void CEndoScopeMFCDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case SHOWTIME:
		ShowCurrentDateTime();
		break;
	case SHOWFPS:
		//cout << "FPS: " << g_FPS << endl;
		g_FPS = 0;
		break;
	case WHITEBALANCE:
		if (m_badjwhite)
		{
			GetDlgItem(IDC_STATIC_INFOBOX)->SetWindowText(_T("白平衡已完成！"));
			GetDlgItem(IDC_STATIC_INFOBOX)->ShowWindow(SW_SHOW);
			Sleep(2000);
			GetDlgItem(IDC_STATIC_INFOBOX)->ShowWindow(SW_HIDE);
			KillTimer(WHITEBALANCE);
		}

		
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

/*
*功能：初始化线程,没有使用
*/
void CEndoScopeMFCDlg::InitializeThread()
{
	//录屏线程，创建挂起
	//unsigned threadID;//录像线程
	//m_hthread = (HANDLE)_beginthreadex(NULL,0,ScreenCAPThreadProc,this,CREATE_SUSPENDED,&threadID);
	//启动显示线程
	//m_hthreadprocess = AfxBeginThread(ProcessImage, &m_dwcap, CREATE_SUSPENDED);
	//创建接收线程
	m_brecvvideo = true;
	CWinThread *pRecvThread = AfxBeginThread(RecvVideoThreadFunc,this);
	if(NULL == pRecvThread)
	{
		TRACE(_T("创建接收线程出错\n"));
	}
	//创建显示线程
	m_bshowvideo = true;
	CWinThread *pShowThread = AfxBeginThread(ShowVideoThreadFunc,this);
	if(NULL == pShowThread)
	{
		TRACE(_T("创建显示线程出错\n"));
	}
}

UINT CEndoScopeMFCDlg::RecvVideoThreadFunc(LPVOID lpParameter)
{
	CEndoScopeMFCDlg *pObj = (CEndoScopeMFCDlg*)lpParameter;
	BYTE *RecvBuffer = new BYTE[IMGWIDTH*IMGHEIGHT*3];
	int datalen = 0;
	int readlen = 0;
	while(pObj->m_brecvvideo)
	{
		readlen = ReadPackQueue(RecvBuffer,&datalen);
		if(readlen < 0 || datalen  <= 0)
		{
			Sleep(1);
			continue;
		}
		if (pObj->m_badjwhite)
		{
			pObj->m_bwhiteok = false;
			pObj->CalWhilteLamda(RecvBuffer);
			continue;
		}
		//白平衡调整
		pObj->AdjWhiteBalance(RecvBuffer, IMGWIDTH, IMGHEIGHT, &(pObj->m_src.data));

		//memcpy(pObj->m_src.data,RecvBuffer,IMGSIZE);
		//grayWB->balanceWhite(m_src,m_src);
		if (pObj->m_src.empty())
		{
			continue;
			//delete pbuftest;
			//delete buf;
		}
		else
		{
			WriteQueue(pObj->m_src.data,IMGSIZE);
		}

		
		////镜像处理
		//flip(pObj->m_src, pObj->m_src, 0);
		////插值放大
		//if (pObj->m_dZoom>1.0)
		//{
		//	Mat t_img ;
		//	WaitForSingleObject(g_resizeevent, INFINITE);
		//	resize(pObj->m_src, t_img, cvSize(pObj->roi_w, pObj->roi_h), INTER_NEAREST);
		//	t_img(Rect(pObj->roi_c, pObj->roi_r, IMGWIDTH, IMGHEIGHT)).copyTo(pObj->m_src);
		//	SetEvent(g_resizeevent);
		//}

		//resize(pObj->m_src,pObj->m_dst,cvSize(1536,864),INTER_NEAREST);
		//imshow("view", pObj->m_dst);
		//////imwrite("test.jpg",m_dst);
		//if (pObj->m_bwritevideo)
		//{
		//	WriteQueue(pObj->m_src.data,IMGSIZE);
		//}		
		//if(pObj->m_bwritepicture)
		//{
		//	unsigned char* pbuf = new unsigned char[IMGSIZE];
		//	memset(pbuf, 0, IMGSIZE);
		//	memcpy(pbuf, pObj->m_src.data, IMGSIZE);
		//	//	WaitForSingleObject(g_picevent, INFINITE);
		//	pObj->m_pPicList.push_back(pbuf);//操作队列
		//	SetEvent(g_picevent);
		//	pObj->m_bwritepicture = false;
		//}
		//waitKey(16);
		//g_FPS += 1;
		Sleep(1);
	}

	delete[] RecvBuffer;
	RecvBuffer = NULL;
	return 0;

}

UINT CEndoScopeMFCDlg::ShowVideoThreadFunc(LPVOID lpParameter)
{
	CEndoScopeMFCDlg *pObj = (CEndoScopeMFCDlg*)lpParameter;
	int ret =0;
	int len = 0;
	int savepicnum = 0;
	//InitYUVQueue();
	Mat m_showrc = Mat(Size(IMGWIDTH, IMGHEIGHT), CV_8UC3);
	while(pObj->m_bshowvideo)
	{
		ret = ReadQueue(m_showrc.data, &len);
		if (len <= 0 || ret < 0){
			Sleep(1);
			continue;
		}
		flip(m_showrc, m_showrc, 0);

		if (pObj->m_bwritevideo)
		{
			WriteYUVQueue(m_showrc.data,IMGSIZE);
		}		
		if(pObj->m_bwritepicture)
		{
			savepicnum++;
			if(savepicnum<11)
			{
				WritePicQueue(m_showrc.data,IMGSIZE);
			}
			else
			{
				savepicnum = 0;
				pObj->m_bwritepicture = false;
			}
		}
		//插值放大
		if (pObj->m_dZoom>1.0)
		{
			Mat t_img ;
			WaitForSingleObject(g_resizeevent, INFINITE);
			resize(m_showrc, t_img, cvSize(pObj->roi_w, pObj->roi_h), INTER_NEAREST);
			t_img(Rect(pObj->roi_c, pObj->roi_r, IMGWIDTH, IMGHEIGHT)).copyTo(m_showrc);
			SetEvent(g_resizeevent);
		}

		resize(m_showrc,pObj->m_dst,cvSize(1536,864),INTER_NEAREST);

		if(!pObj->m_bshowpause)
		{
			cv::imshow("view", pObj->m_dst);
		}
		//imwrite("test.jpg",m_dst);

		waitKey(1);
		g_FPS += 1;
		//Sleep(1);

	}
	m_showrc.release();
	return 0;
}

#if 0
UINT CEndoScopeMFCDlg::ProcessImage(LPVOID lpParameter)
{
	CEDSCapture *pCapture = (CEDSCapture*)lpParameter;
	while (g_bprocthread)
	{
		BYTE* pBuffer=pCapture->GetFrame();
		if (pBuffer)
		{
			//处理
			//AdjWhiteBalance(pBuffer,lamda,IMGWIDTH,IMGHEIGHT)
			Mat src = Mat::zeros(Size(IMGWIDTH, IMGHEIGHT), CV_8UC3);
			memcpy(src.data, pBuffer, IMGSIZE);
			flip(src, src, 0);
			imshow("view", src);
			waitKey(1);
			delete[] pBuffer;//这个地方不让释放
			pBuffer = NULL;
		}
	}
	return 0;
}
#endif
UINT CEndoScopeMFCDlg::ScreenCAPThreadProc(LPVOID lpParameter)
{
#if 1
	CEndoScopeMFCDlg* pdlg = (CEndoScopeMFCDlg*)lpParameter;
	int ret =0;
	int len = 0;
	Mat img = Mat(IMGHEIGHT, IMGWIDTH, CV_8UC3);
	//cout << "worker thread:"<<std::this_thread::get_id() << endl;
	while (pdlg->m_bstartsave)
	{
		//BYTE* t_pbuf=pdlg->GetFrame();
		ret = ReadYUVQueue(img.data, &len);
		if (len <= 0 || ret < 0){
			Sleep(1);
			continue;
		}
		if (!pdlg->m_vwriter.isOpened())
		{
			continue;
		}
		if (!img.empty())
		{
			pdlg->m_vwriter.write(img);
		}
	}
	if (pdlg->m_vwriter.isOpened())
	{
		pdlg->m_vwriter.release();
	}
#endif
	return 0;
}
//屏幕数据获取算法
char* CEndoScopeMFCDlg::GetScreenDataGDI()
{
	char *screenCaptureData = new char[m_nScreenWidth*m_nScreenHeight * 4];
	memset(screenCaptureData, 0, m_nScreenWidth);

	if (BitBlt(m_hBmpFileDC, 0, 0, m_nScreenWidth, m_nScreenHeight, m_hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT))
	{
		GetBitmapBits(m_hBitmap, m_nScreenWidth*m_nScreenHeight * 4, screenCaptureData);
	}
	return screenCaptureData;
}
//内镜信息  
void CEndoScopeMFCDlg::ShowEndoscopeInfo(BYTE * pInfo, int nSize)
{
	if (pInfo)
	{

		BYTE* pTmp = new BYTE[nSize + 1];
		memset(pTmp, 0, nSize + 1);
		memcpy(pTmp, pInfo, nSize);
		CString strNum, strDate, strLen;
		strNum.Format(_T("%s""%c""%c""%c""%c"), "ENDE-", pTmp[9], pTmp[7], pTmp[8], pTmp[6]);
		//strNum = "ENDE-310D";
		GetDlgItem(IDC_ENDOSCOPEID_INFO_STATIC)->SetWindowText(strNum);
		strDate.Format(_T("%c""%c""%c""%c""-""%c""%c""-""%c""%c"), pTmp[10], pTmp[11], pTmp[12], pTmp[13], pTmp[14], pTmp[15], pTmp[16], pTmp[17]);
		//GetDlgItem(IDC_ENDOSCOPEDATA_INFO_STATIC)->SetWindowText(strDate);//出厂日期没有
		//剩余使用次数
		//strNum.Format(_T("%c"), pTmp[18] );
		//GetDlgItem(IDC_DISPOSABLE_INFO_STATIC)->SetWindowText(_T("是"));
		if (255 == pTmp[18])
		{
			GetDlgItem(IDC_ENDOSCOPEID_INFO_STATIC)->SetWindowText(_T("ENDE-240"));
			GetDlgItem(IDC_DISPOSABLE_INFO_STATIC)->SetWindowText(_T("否"));
		}
		else
		{
			GetDlgItem(IDC_DISPOSABLE_INFO_STATIC)->SetWindowText(_T("是"));
		}

		SAFE_DELETE_ARRAY(pTmp);
	}
	else
	{
		CString strNum = _T("未插入");
		GetDlgItem(IDC_ENDOSCOPEID_INFO_STATIC)->SetWindowText(strNum);
		GetDlgItem(IDC_DISPOSABLE_INFO_STATIC)->SetWindowText(_T(""));
	}
}

LRESULT CEndoScopeMFCDlg::OnCtlData(WPARAM wParam, LPARAM lParam)
{
	
	CRgn roundrgn;
	unsigned char* pBuf = (unsigned char*)wParam;


	//CString temp(pBuf);
	//AfxMessageBox(temp);
	//根据说明串口的字节数一般是13-15个字节，第5和6个两个字节表示存储地址，其中第5个字节是00
	switch (pBuf[5])
	{
	case 0x01:
		switch (pBuf[8])
		{
		case 0x0A://录入病人信息
			EnterPatientInfo();
			break;
		case 0x0B://录像
			SaveVideo();
			break;
		case 0x0C://存储到外部设备
			MoveDatatoUSBDevice();
			break;
		default:
			break;
		}
		break;
	case 0x25://测光模式
		SetModeofMeterLight(pBuf);//测光模式
		break;
	case 0x26://B变换
		//printf("*******************change B*****************");
		ChangeImageRB(pBuf,1);
		break;
	case 0x27://R值变换
		//printf("*******************change R*****************");
		ChangeImageRB(pBuf, 2);
		break;
	case 0x28://放大
		if (0x83 == pBuf[3] && 0x00 == pBuf[4])
		{
			VideoZoom();
		}
		break;
	case 0x29://设置轮廓
		if (pBuf[8] == 0x0A)//圆角
		{
			m_nPicWindowShape = 0;
		}
		else if (pBuf[8] == 0x0B)//方角
		{
			m_nPicWindowShape = 1;
		}
		SetPicWidowShape();
		break;
	case 0x07://恢复出厂设置
		RestoreFactorySetting();
		break;
	case 0x2A://光源开关
		LightControler(pBuf);
		break;
	case 0x23://冻结
		ShotPicWindow();
		break;
	case 0x24:
		UpdateWhiteBalance();
		//cout<<"updatebalance control";
		break;
	case 0xB0://设置图像
		ImageSetting(pBuf);
		break;
	case 0x55://内镜信息
		ShowEndoscopeInfo(pBuf, 19);
		break;
	default:
		break;
	}
	return(Default());
}
/*
*	设置清晰度、亮度、对比度、饱和度、曝光、增益调节
*	输入：串口字符串
*	输出：无，内部调用函数
*/
void CEndoScopeMFCDlg::ImageSetting(unsigned char* pbuf)
{
	switch (pbuf[8])
	{
		case 0x03:
			//清晰度增加
			if (g_nSharpness < 10)
				SharpnessAdd();
			SetConfig(_T("SHARPNESS"), g_nSharpness);
			g_bOneSharpen = TRUE;
			break;
		case 0x02:
			//清晰度减小
			if (g_nSharpness > 0)
				SharpnessSub();

			SetConfig(_T("SHARPNESS"), g_nSharpness);
			g_bOneSharpen = TRUE;
			break;
		case 0x05:
			//亮度增加
			if (g_nBrightness < 10) BrightnessAdd();
			break;
		case 0x04:
			//亮度减小
			if (g_nBrightness > -10) BrightnessSub();
			break;
		case 0x07:
			//对比度增加
			if (g_nContrast < 10) ContrastAdd();
			break;
		case 0x06:
			//对比度减小
			if (g_nContrast > 0) ContrastSub();
			break;
		case 0x09:
			//饱和度增加
			if (g_nSaturation < 10) SaturationAdd();
			break;
		case 0x08:
			//饱和度减小
			if (g_nSaturation > 0) SaturationSub();
			break;
		case 0x0D:
			//增益增加
			if (g_nGain < 10) GainAdd();
			break;
		case 0x0C://调节控制板
			if (g_nGain > 0) GainSub();
			break;
		case 0x0B://曝光增加
			if(g_nExposure<0)ExposeAdd();
			break;
		case 0x0A://曝光减少
			if(g_nExposure>-10)
				ExposeSub();
			break;
		default:
			break;
		}
}
void CEndoScopeMFCDlg::UpdateWhiteBalance()
{
	//if (m_bsetuplamda)
	//{
		//g_pPLC->Brightness10();
		//Sleep(5);

		GetDlgItem(IDC_STATIC_INFOBOX)->SetWindowText(_T("正在进行白平衡..."));
		GetDlgItem(IDC_STATIC_INFOBOX)->ShowWindow(SW_SHOW);
		g_pPLC->WhiteBalance();
		Sleep(2000);
		//m_dwcap.SetupLamdaRGB();
		m_badjwhite = true;
		SetTimer(WHITEBALANCE, 800, NULL);
		//m_bsetuplamda = false;
	//}
	//else
	//{
	//	m_bsetuplamda = true;
	//}
}
void CEndoScopeMFCDlg::LightControler(unsigned char*pInfo)
{
	if (pInfo[8]== 0x0C)
	{
		if (g_bIsLightOpen)//初始化启动是FALSE
		{
			//冷光源关
			g_pPLC->CloseLight();
			g_nLBrightness = 0;
			g_pPLC->Light(g_nLBrightness);
			g_bIsLightOpen = FALSE;
		}
		else
		{
			//冷光源开
			g_pPLC->OpenLight();
			g_nLBrightness = 100;
			g_pPLC->Light(g_nLBrightness);
			g_bIsLightOpen = TRUE;
		}
	}
}

void CEndoScopeMFCDlg::RestoreFactorySetting()//出厂设置
{
	GetDlgItem(IDC_STATIC_INFOBOX)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_INFOBOX)->SetWindowText(_T("正在恢复出厂设置..."));
	GetDlgItem(IDC_STATIC_INFOBOX)->ShowWindow(SW_SHOW);
	Sleep(3000);
	GetDlgItem(IDC_STATIC_INFOBOX)->ShowWindow(SW_HIDE);
	//1.相机亮度恢复
	int nBrightness = GetPrivateProfileInt(_T("FACTORY"), _T("BRIGHTNESS"), 1, g_ConfigPath);
	if((g_nBrightness<-10)||(g_nBrightness>10))
	{
		nBrightness=0;
	}
	g_pPLC->Brightness5();
	g_nBrightness = nBrightness;
	Sleep(20);
	//2.相机对比度恢复
	int nContrast = GetPrivateProfileInt(_T("FACTORY"), _T("CONTRAST"), 1, g_ConfigPath);
	if((nContrast<0)||(nContrast>10))
	{
		nContrast=0;
	}
	g_pPLC->Contrast(nContrast);
	g_nContrast = nContrast;
	Sleep(20);
	//3.相机饱和度恢复
	int nSaturation = GetPrivateProfileInt(_T("FACTORY"), _T("SATURATION"), 1, g_ConfigPath);
	if((nSaturation<0)||(nSaturation>10))
	{
		nSaturation=5;
	}
	g_pPLC->Saturation(nSaturation);
	g_nSaturation = nSaturation;
	Sleep(20);
	//13.色调R B
	int nRed = GetPrivateProfileInt(_T("FACTORY"), _T("HUE_R"), 1, g_ConfigPath);
	if((nRed<-9)||(nRed>9))
	{
		nRed = 0;
	}
	g_pPLC->R(nRed);
	int pos = m_strRB.Find('R');
	m_strRB.SetAt(++pos, '+');
	m_strRB.SetAt(++pos, '0');
	GetDlgItem(IDC_IMGCLR_INFO_STATIC)->SetWindowText(m_strRB);
	g_nRed = nRed;
	Sleep(20);

	int nBlue = GetPrivateProfileInt(_T("FACTORY"), _T("HUE_B"), 1, g_ConfigPath);
	if((nBlue<-9)||(nBlue>9))
	{
		nBlue = 0;
	}
	g_pPLC->B(nBlue);
	pos = m_strRB.Find('B');
	m_strRB.SetAt(++pos, '+');
	m_strRB.SetAt(++pos, '0');
	GetDlgItem(IDC_IMGCLR_INFO_STATIC)->SetWindowText(m_strRB);
	g_nBlue = nBlue;
	Sleep(20);
	//4.相机增益恢复
	int nGain = GetPrivateProfileInt(_T("FACTORY"), _T("GAIN"), 1, g_ConfigPath);
	if((nGain<0)||(nGain>10))
	{
		nGain=0;
	}
	g_pPLC->Gain(nGain);
	g_nGain = nGain;
	Sleep(20);
	//5.相机曝光恢复
	int nExposure = GetPrivateProfileInt(_T("FACTORY"), _T("EXPOSURE"), 1, g_ConfigPath);
	if((nExposure>0)||(nExposure<-10))
	{
		nExposure=0;
	}
	g_pPLC->Exposure2();
	g_nExposure = nExposure;
	//g_nSharpness = 6;
	Sleep(20);
	//6.白平衡设置
	//7.色调设置
	//8.清晰度设置
	//int nSharpness=GetPrivateProfileInt(_T("FACTORY"),_T("SHARPNESS"),1,g_ConfigPath);
	//if((nSharpness<0)||(nSharpness>15))
	//{
	//	nSharpness=4 ;
	//}
	//g_pPLC->Sharpness(nSharpness);
	//Sleep(5);
	//9.伽马设置
	//10.测光模式
	int nMetering = GetPrivateProfileInt(_T("FACTORY"), _T("ADJBRIGHT"), 1, g_ConfigPath);
	if(0 == nMetering)
	{
		g_nMetering = 0;
		g_pPLC->AverageMetering();
		GetDlgItem(IDC_TESTLIGHT_INFO_STATIC)->SetWindowText(_T("平均测光"));
	}
	else if(1 == nMetering)
	{
		g_nMetering = 1;
		g_pPLC->PeakMetering();
		GetDlgItem(IDC_TESTLIGHT_INFO_STATIC)->SetWindowText(_T("峰值测光"));
	}
	Sleep(20);
	//11.轮廓
	int nShowShape = GetPrivateProfileInt(_T("FACTORY"), _T("CONTUOR"), 0, g_ConfigPath);
	if(0 == nShowShape)
	{
		m_nPicWindowShape = 0;//设置成圆角

	}
	else if(1 == nShowShape)
	{
		m_nPicWindowShape = 1;//设置成方角
	}
	SetPicWidowShape();
	Sleep(20);
	//12.放大
	g_pPLC->Zoom1();
	GetDlgItem(IDC_ENLARGE_INFO_STATIC)->SetWindowText(_T("1.0倍"));
	SetZoom(1);
	m_nZoomIn = 1;
	Sleep(20);

	GetDlgItem(IDC_STATIC_INFOBOX)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_INFOBOX)->SetWindowText(_T("恢复出厂设置完成"));
	GetDlgItem(IDC_STATIC_INFOBOX)->ShowWindow(SW_SHOW);
	Sleep(3000);
	GetDlgItem(IDC_STATIC_INFOBOX)->ShowWindow(SW_HIDE);

}

void CEndoScopeMFCDlg::SetModeofMeterLight(unsigned char*pbuf)
{
	cout<<"get signal"<<endl;
	if (pbuf[8] == 0x0A)
	{
		//平均测光
		g_nMetering = 0;
		g_pPLC->AverageMetering();
		GetDlgItem(IDC_TESTLIGHT_INFO_STATIC)->SetWindowText(_T("平均测光"));
	}
	else if (0x0B == pbuf[8])
	{
		//峰值测光
		g_nMetering = 1;
		g_pPLC->PeakMetering();
		GetDlgItem(IDC_TESTLIGHT_INFO_STATIC)->SetWindowText(_T("峰值测光"));
	}
}

void CEndoScopeMFCDlg::ChangeImageRB(unsigned char *pbuf,int flag)
{
	//printf("1%d\n",g_nBlue);
	if (flag==1)
	{
		if (pbuf[8] == 0x0A)
		{
			//-B值
			if (g_nBlue > -9)
			{
				g_nBlue--;
				//SetConfig(_T("HUE_B"), g_nBlue);
				g_pPLC->B(g_nBlue);//LJ
				char B;
				B = '0' + abs(g_nBlue);
				if (g_nBlue >= 0)
				{
					int pos = m_strRB.Find('B');
					m_strRB.SetAt(++pos, '+');
					m_strRB.SetAt(++pos, B);
				}
				else
				{
					int pos = m_strRB.Find('B');
					m_strRB.SetAt(++pos, '-');
					m_strRB.SetAt(++pos, B);
				}
				//printf("2%d\n",g_nBlue);
				GetDlgItem(IDC_IMGCLR_INFO_STATIC)->SetWindowText(m_strRB);
				if (0 == g_nBlue)
				{
					g_pPLC->B(g_nBlue);
				}
			}
			g_bOneB = TRUE;
		}
		else if (pbuf[8] == 0x0B)
		{
			//+B值
			//printf("3%d\n",g_nBlue);
			if (g_nBlue < 9)
			{
				g_nBlue++;
				//SetConfig(_T("HUE_B"), g_nBlue);
				g_pPLC->B(g_nBlue);//LJ
				char B;
				B = '0' + abs(g_nBlue);
				if (g_nBlue >= 0)
				{
					int pos = m_strRB.Find('B');
					m_strRB.SetAt(++pos, '+');
					m_strRB.SetAt(++pos, B);
				}
				else
				{
					int pos = m_strRB.Find('B');
					m_strRB.SetAt(++pos, '-');
					m_strRB.SetAt(++pos, B);
				}
				//printf("4%d\n",g_nBlue);
				GetDlgItem(IDC_IMGCLR_INFO_STATIC)->SetWindowText(m_strRB);
				if (0 == g_nBlue)
				{
					g_pPLC->B(g_nBlue);
				}
			}
			g_bOneB = TRUE;
		}
	}
	else if (flag==2)
	{
		if (pbuf[8] == 0x0A)
		{
			//-R值
			if (g_nRed > -9)
			{
				g_nRed--;
				//SetConfig(_T("HUE_R"), g_nRed);
				g_pPLC->R(g_nRed);//LJ
				char R;
				R = '0' + abs(g_nRed);

				if (g_nRed >= 0)
				{
					int pos = m_strRB.Find('R');
					m_strRB.SetAt(++pos, '+');
					m_strRB.SetAt(++pos, R);
				}
				else
				{
					int pos = m_strRB.Find('R');
					m_strRB.SetAt(++pos, '-');
					m_strRB.SetAt(++pos, R);
				}
				GetDlgItem(IDC_IMGCLR_INFO_STATIC)->SetWindowText(m_strRB);

				if (0 == g_nRed)
				{
					g_pPLC->R(g_nRed);
				}
			}
			g_bOneR = TRUE;
		}
		else if (pbuf[8] == 0x0B) 
		{
			//+R值
			if (g_nRed < 9)
			{
				g_nRed++;
				//SetConfig(_T("HUE_R"), g_nRed);
				g_pPLC->R(g_nRed);//LJ
				char R;
				R = '0' + abs(g_nRed);

				if (g_nRed >= 0)
				{
					char B;
					B = '0' + g_nRed;
					int pos = m_strRB.Find('R');
					m_strRB.SetAt(++pos, '+');
					m_strRB.SetAt(++pos, R);
				}
				else
				{
					int pos = m_strRB.Find('R');
					m_strRB.SetAt(++pos, '-');
					m_strRB.SetAt(++pos, R);
				}
				GetDlgItem(IDC_IMGCLR_INFO_STATIC)->SetWindowText(m_strRB);

				if (0 == g_nRed)
				{
					g_pPLC->R(g_nRed);
				}
			}
			g_bOneR = TRUE;
		}
	}
}

void CEndoScopeMFCDlg::SaveVideo()
{
//	static BOOL bStart=FALSE;

	if (!bSaveVideoStart)//启动线程
	{
		g_bVideo = TRUE;
		//m_dwcap.StartWriteVideo();
		//InitQueue();
		m_bstartsave = true;
		int codec = VideoWriter::fourcc('M', 'J', 'P', 'G');
		string t_imgpath = GetSaveImgPath(0);
		bool isok = m_vwriter.open(t_imgpath, codec,30, Size(IMGWIDTH, IMGHEIGHT));
		if (isok)
		{
			pThreadVideo = AfxBeginThread(ScreenCAPThreadProc, this);
		}
		m_bwritevideo = true;
		g_pPLC->NowVideo();
		GetDlgItem(IDC_RECORD_INFO_STATIC)->SetWindowText(_T("正在录制"));
	}
	else
	{
		m_bwritevideo = false;		
		//m_dwcap.StopWriteVideo();
		g_pPLC->StopVideo();
		g_bVideo = FALSE;
		m_bstartsave = false;
		resetQueue();
		GetDlgItem(IDC_RECORD_INFO_STATIC)->SetWindowText(_T("录制完成"));
	}
	this->SetFocus();//这个很重要，要将当前 窗口激活状态
	bSaveVideoStart = !bSaveVideoStart;
}
void CEndoScopeMFCDlg::MoveDatatoUSBDevice()
{   
	if(!g_bStop)
	{
		g_bStop = TRUE;
	}
	else
	{
		return;
	}
	SaveToPeripherals();
	g_bStop = FALSE;
}

void CEndoScopeMFCDlg::EnterPatientInfo()
{
	if (m_bInputInfo)
	{
		//cout << "input" << endl;
		GetDlgItem(IDC_NAME_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_ID_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_SEX_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_AGE_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_DOCTOR_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_ID_EDIT)->SetFocus();
		g_pPLC->NowEnter();
	}
	else
	{
		//cout << "cant input" << endl;
		GetDlgItem(IDC_NAME_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_ID_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_SEX_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_AGE_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_DOCTOR_EDIT)->EnableWindow(FALSE);
		g_pPLC->OverEnter();
		//m_bInputInfo = true;
		//GetDlgItem(IDC_STILL)->SetFocus();//这个很重要，要将当前 窗口激活状态
		this->SetFocus();
		
	}
	m_bInputInfo=!m_bInputInfo;

}
//亮度+
void CEndoScopeMFCDlg::BrightnessAdd()
{
	int nBrightness= ++g_nBrightness;
	
	SetConfig(_T("BRIGHTNESS"), nBrightness);
	//}
	switch (nBrightness)
	{
	case -10:
		g_pPLC->Brightness20();
		//g_nExposure = -10;
		break;
	case -9:
		g_pPLC->Brightness19();
		//g_nExposure = -10;
		break;
	case -8:
		g_pPLC->Brightness18();
		//g_nExposure = -9;
		break;
	case -7:
		g_pPLC->Brightness17();
		//g_nExposure = -9;
		break;
	case -6:
		g_pPLC->Brightness16();
		//g_nExposure = -8;
		break;
	case -5:
		g_pPLC->Brightness15();
		//g_nExposure = -8;
		break;
	case -4:
		g_pPLC->Brightness14();
		//g_nExposure = -7;
		break;
	case -3:
		g_pPLC->Brightness13();
		//g_nExposure = -7;
		break;
	case -2:
		g_pPLC->Brightness12();
		//g_nExposure = -6;
		break;
	case -1:
		g_pPLC->Brightness11();
		//g_nExposure = -6;
		break;
	case 0:
		g_pPLC->Brightness0();
		//g_nExposure = -5;
		break;
	case 1:
		g_pPLC->Brightness1();
		//g_nExposure = -5;
		break;
	case 2:
		g_pPLC->Brightness2();
		//g_nExposure = -4;
		break;
	case 3:
		g_pPLC->Brightness3();
		//g_nExposure = -4;
		break;
	case 4:
		g_pPLC->Brightness4();
		//g_nExposure = -3;
		break;
	case 5:
		g_pPLC->Brightness5();
		//g_nExposure = -3;
		break;
	case 6:
		g_pPLC->Brightness6();
		//g_nExposure = -2;
		break;
	case 7:
		g_pPLC->Brightness7();
		//g_nExposure = -2;
		break;
	case 8:
		g_pPLC->Brightness8();
		//g_nExposure = -1;
		break;
	case 9:
		g_pPLC->Brightness9();
		//g_nExposure = -1;
		break;
	case 10:
		g_pPLC->Brightness10();
		//g_nExposure = 0;
		break;
	default:
		break;
	}
}
//亮度-
void CEndoScopeMFCDlg::BrightnessSub()
{
	int nBrightness= --g_nBrightness;
	
	SetConfig(_T("BRIGHTNESS"), nBrightness);
	switch (nBrightness)
	{
	case -10:
		g_pPLC->Brightness20();
		//g_nExposure = -10;
		break;
	case -9:
		g_pPLC->Brightness19();
		//g_nExposure = -10;
		break;
	case -8:
		g_pPLC->Brightness18();
		//g_nExposure = -9;
		break;
	case -7:
		g_pPLC->Brightness17();
		//g_nExposure = -9;
		break;
	case -6:
		g_pPLC->Brightness16();
		//g_nExposure = -8;
		break;
	case -5:
		g_pPLC->Brightness15();
		//g_nExposure = -8;
		break;
	case -4:
		g_pPLC->Brightness14();
		//g_nExposure = -7;
		break;
	case -3:
		g_pPLC->Brightness13();
		//g_nExposure = -7;
		break;
	case -2:
		g_pPLC->Brightness12();
		//g_nExposure = -6;
		break;
	case -1:
		g_pPLC->Brightness11();
		//g_nExposure = -6;
		break;
	case 0:
		g_pPLC->Brightness0();
		//g_nExposure = -5;
		break;
	case 1:
		g_pPLC->Brightness1();
		//g_nExposure = -5;
		break;
	case 2:
		g_pPLC->Brightness2();
		//g_nExposure = -4;
		break;
	case 3:
		g_pPLC->Brightness3();
		//g_nExposure = -4;
		break;
	case 4:
		g_pPLC->Brightness4();
		//g_nExposure = -3;
		break;
	case 5:
		g_pPLC->Brightness5();
		//g_nExposure = -3;
		break;
	case 6:
		g_pPLC->Brightness6();
		//g_nExposure = -2;
		break;
	case 7:
		g_pPLC->Brightness7();
		//g_nExposure = -2;
		break;
	case 8:
		g_pPLC->Brightness8();
		//g_nExposure = -1;
		break;
	case 9:
		g_pPLC->Brightness9();
		//g_nExposure = -1;
		break;
	case 10:
		g_pPLC->Brightness10();
		//g_nExposure = 0;
		break;
	default:
		break;
	}
}
//对比度+
void CEndoScopeMFCDlg::ContrastAdd()
{
	//printf("contrast  %d",g_nContrast);
	int nContrast = ++g_nContrast;
	g_pPLC->Contrast(nContrast);
}

//对比度-
void CEndoScopeMFCDlg::ContrastSub()
{
	//printf("contrast  %d",g_nContrast);
	int nContrast = --g_nContrast;

	g_pPLC->Contrast(nContrast);
}
//饱和度+
void CEndoScopeMFCDlg::SaturationAdd()
{
	int nSaturation = ++g_nSaturation;
	g_pPLC->Saturation(nSaturation);
}
//饱和度-
void CEndoScopeMFCDlg::SaturationSub()
{

	int nSaturation = --g_nSaturation;
	g_pPLC->Saturation(nSaturation);
}
//色调+
void CEndoScopeMFCDlg::HueAdd()
{

}
//色调-
void CEndoScopeMFCDlg::HueSub()
{

}
//清晰度+
void CEndoScopeMFCDlg::SharpnessAdd()
{
	g_nSharpness++;
	g_nExposure++;
	g_pPLC->Sharpness(g_nSharpness);
}
//清晰度-
void CEndoScopeMFCDlg::SharpnessSub()
{
	g_nSharpness--;
	g_nExposure--;
	g_pPLC->Sharpness(g_nSharpness);
}
//增益+
void CEndoScopeMFCDlg::GainAdd()
{
	int nGain = ++g_nGain;
	BOOL ok=g_pPLC->Gain(nGain);

}
//增益-
void CEndoScopeMFCDlg::GainSub()
{
	int nGain = --g_nGain;
	BOOL ok=g_pPLC->Gain(nGain);

}
bool CEndoScopeMFCDlg::ExposeAdd()
{
	int nexpose = ++g_nExposure;
	switch (nexpose)
	{
	case -10:
		g_pPLC->Exposure10();
		g_nSharpness = 0;
		break;
	case -9:
		g_pPLC->Exposure9();
		g_nSharpness = 1;
		break;
	case -8:
		g_pPLC->Exposure8();
		g_nSharpness = 2;
		break;
	case -7:
		g_pPLC->Exposure7();
		g_nSharpness = 3;
		break;
	case -6:
		g_pPLC->Exposure6();
		g_nSharpness = 4;
		break;
	case -5:
		g_pPLC->Exposure5();
		g_nSharpness = 5;
		break;
	case -4:
		g_pPLC->Exposure4();
		g_nSharpness = 6;
		break;
	case -3:
		g_pPLC->Exposure3();
		g_nSharpness = 7;
		break;
	case -2:
		g_pPLC->Exposure2();
		g_nSharpness = 8;
		break;
	case -1:
		g_pPLC->Exposure1();
		g_nSharpness = 9;
		break;
	case 0:
		g_pPLC->Exposure0();
		g_nSharpness = 10;
		break;
	default:
		break;
	}
	return true;
}
bool CEndoScopeMFCDlg::ExposeSub()
{
	int nexpose = --g_nExposure;
	
	switch (nexpose)
	{
	case -10:
		g_pPLC->Exposure10();
		g_nSharpness = 0;
		break;
	case -9:
		g_pPLC->Exposure9();
		g_nSharpness = 1;
		break;
	case -8:
		g_pPLC->Exposure8();
		g_nSharpness = 2;
		break;
	case -7:
		g_pPLC->Exposure7();
		g_nSharpness = 3;
		break;
	case -6:
		g_pPLC->Exposure6();
		g_nSharpness = 4;
		break;
	case -5:
		g_pPLC->Exposure5();
		g_nSharpness = 5;
		break;
	case -4:
		g_pPLC->Exposure4();
		g_nSharpness = 6;
		break;
	case -3:
		g_pPLC->Exposure3();
		g_nSharpness = 7;
		break;
	case -2:
		g_pPLC->Exposure2();
		g_nSharpness = 8;
		break;
	case -1:
		g_pPLC->Exposure1();
		g_nSharpness = 9;
		break;
	case 0:
		g_pPLC->Exposure0();
		g_nSharpness = 10;
		break;
	default:
		break;
	}
	return true;
}
bool CEndoScopeMFCDlg::MenuStart()//菜单开
{ 
	bool bMenuStart = false;
	bMenuStart = g_pPLC->MenuStart();
	return bMenuStart;
}
bool CEndoScopeMFCDlg::MenuStop()//菜单关
{
	bool bMenuStop = false;
	bMenuStop = g_pPLC->MenuStop();
	return bMenuStop;
}
bool CEndoScopeMFCDlg::MenuUp()//菜单上
{
	bool bMenuUp = false;
	bMenuUp = g_pPLC->MenuUp();
	return bMenuUp;
}
bool CEndoScopeMFCDlg::MenuDown()
{
	bool bMenuDown = false;
	bMenuDown = g_pPLC->MenuDown();
	return bMenuDown;
}
bool CEndoScopeMFCDlg::MenuLeft()
{
	bool bMenuLeft = false;
	bMenuLeft = g_pPLC->MenuLeft();
	return bMenuLeft;
}
bool CEndoScopeMFCDlg::MenuRight()
{
	bool bMenuRight = false;
	bMenuRight = g_pPLC->MenuRight();
	return bMenuRight;
}
bool CEndoScopeMFCDlg::MenuEnter()
{
	bool bMenuEnter = false;
	bMenuEnter = g_pPLC->MenuEnter();
	return bMenuEnter;
}
BOOL CutDirFileAPI(CString strSrcPath, CString strDesPath)
{
	//剪切文件

	if (strSrcPath.GetAt(strSrcPath.GetLength() - 1) == '\\' || strSrcPath.GetAt(strSrcPath.GetLength() - 1) == '/')
	{
		strSrcPath = strSrcPath.Left(strSrcPath.GetLength() - 1);
	}
	if (strDesPath.GetAt(strDesPath.GetLength() - 1) == '\\' || strDesPath.GetAt(strDesPath.GetLength() - 1) == '/')
	{
		strDesPath = strDesPath.Left(strDesPath.GetLength() - 1);
	}

	char szFrom[MAX_PATH] = { 0 };
	char szTo[MAX_PATH] = { 0 };
	strSrcPath = strSrcPath + "\\*.*";
	strcpy_s(szFrom, (LPCTSTR)strSrcPath);
	szFrom[strSrcPath.GetLength() + 1] = '\0';
	//strDes=strDes+"\\*.*";
	strcpy_s(szTo, (LPCTSTR)strDesPath);
	szTo[strDesPath.GetLength() + 1] = '\0';
	int nOk = 2000;
	char strTitle[] = "正在拷贝文件,请稍候..."; //进度条标题
	SHFILEOPSTRUCT FileOp;
	const char* From = szFrom;
	const char* To = szTo;
	//FileOp.hwnd = this->m_hWnd;
	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_MOVE; //执行文件剪切
	FileOp.pFrom = From;
	FileOp.pTo = To;
	FileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
	FileOp.hNameMappings = NULL;
	FileOp.lpszProgressTitle = strTitle;
	nOk = SHFileOperation(&FileOp);
	if (FileOp.fAnyOperationsAborted) //防止出现低配置光驱BUG
	{
		nOk = 1;
	}
	if (nOk == 1)
	{
		return false;
	}
	Sleep(200);
	return true;
}
//外设存储
void CEndoScopeMFCDlg::SaveToPeripherals()
{
	if (g_bIsUpan)
	{
		Login login_dlg;
		if (login_dlg.DoModal() == IDOK)
		{
			//g_bCopyFileToFlashDisk = true;
			//判断是否正在录像
			if (g_bVideo)
			{
				m_bwritevideo = false;
				m_bstartsave = false;
				//m_dwcap.StopWriteVideo();
				g_pPLC->StopVideo();
				g_bVideo = FALSE;
				GetDlgItem(IDC_RECORD_INFO_STATIC)->SetWindowText(_T("录制完成"));
				bSaveVideoStart = FALSE;
				//m_bstartsave = false;
			}
			//存储
			g_pPLC->NowSave();
			GetDlgItem(IDC_STORAGE_INFO_STATIC)->SetWindowText(_T("正在存储"));
			CString strPeri, strFileDir;
			strFileDir = g_strSaveDir;
			strPeri = g_decDriver + _T(":\\");
			//BOOL b = CopyFile(strFileDir,strPeri,FALSE); 
			strPeri += _T("内窥镜数据");
			//U盘中没有内窥镜文件夹需要建立一个
			if (!PathIsDirectory(strPeri))
			{
				::CreateDirectory(strPeri, NULL);//创建目录,已有的话不影响  
			}
			m_sdatapath = strPeri+ _T("\\")+m_sdatapath;
			CutDirFileAPI(strFileDir, strPeri);
			g_pPLC->OverSave();
			GetDlgItem(IDC_STORAGE_INFO_STATIC)->SetWindowText(_T("存储完成"));

			//g_bCopyFileToFlashDisk = false;

		}
	}
	else
	{
		//g_pPLC->NotLink();
		//AfxMessageBox("请检查移动存储设备是否正确插入！");

		if (dlgBoxInfo.bHaveDlg)
		{
			dlgBoxInfo.OnClose();
		}
		dlgBoxInfo.DoModal();
	}
}

void CAboutDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}

extern int g_nThresh;
extern int g_nThreshNew;
extern int g_nChangePixel;
extern int g_nChange255;
void CEndoScopeMFCDlg::AdjWhiteBalance(unsigned char* src, int width, int height, unsigned char** dst)
{
	
	int pos = 0,oldpos=0;
	unsigned char *t_dst = *dst;
	for(int i =1;i<2764800;i=i+3)
	{
		src[i] = (int)(src[i] * m_lamda[i]);
	}
	memset(t_dst,0,IMGSIZE);
	memcpy(t_dst,src,IMGSIZE);
}
void CEndoScopeMFCDlg::CalWhilteLamda(BYTE * buf)
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
					data[pos]=(int)val[pos];
					val[pos+1]+=temp[pos+1]/10.;
					data[pos+1]=(int)val[pos+1];
					val[pos+2]+=temp[pos+2]/10.;
					data[pos+2]=(int)val[pos+2];
				}
			}
		}
		//for (int i = 0;i < IMGHEIGHT;++i)
		//{
		//	for (int j = 0;j < IMGWIDTH;++j)
		//	{
		//		int pos=i * IMGWIDTH * 3 + j * 3;
		//		data[pos]=(int)val[pos];
		//		data[pos+1]=(int)val[pos+1];
		//		data[pos+2]=(int)val[pos+2];
		//	}

		//}
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
void CEndoScopeMFCDlg::IniWhilteLamda()
{
	ifstream file("lamda.txt");
	int i=0;
	if (file.fail())
	{
		fill(m_lamda, m_lamda +IMGSIZE,1);
		return;
	}
	else
	{
		string t_buf;
		while(!file.eof())
		{
			getline(file,t_buf);
			m_lamda[i]=atof(t_buf.c_str());
			i++;
		}
		file.clear();
		file.close();
	}

}

void CEndoScopeMFCDlg::SetZoom(double flag)
{
	WaitForSingleObject(g_resizeevent, INFINITE);
	m_dZoom=flag;
	roi_w = IMGWIDTH*flag;
	roi_h = IMGHEIGHT*flag;
	roi_c=(roi_w-IMGWIDTH)/2;
	roi_r=(roi_h-IMGHEIGHT)/2;
	SetEvent(g_resizeevent);
}

BYTE* CEndoScopeMFCDlg::GetFrame()
{
	BYTE* t_buffer = NULL;
	BYTE* x = NULL;
	//cout << "frame thread:" << std::this_thread::get_id() << endl;
	WaitForSingleObject(g_videoevent, INFINITE);
	//数据出列
	if (0<m_pTaskList.size())
	{
		try {
			x = m_pTaskList.front();
			m_pTaskList.pop_front();
			t_buffer = new BYTE[IMGSIZE];
			if (x != NULL)
			{
				memcpy(t_buffer, x, IMGSIZE);
				delete[]x;
				x = NULL;
			}
		}
		catch (const std::exception&err)
		{
			std::cout << err.what() << std::endl;
			return NULL;
		}
	}
	SetEvent(g_videoevent);
	return t_buffer;
}

BYTE* CEndoScopeMFCDlg::GetPicture()
{
	BYTE* t_buffer = NULL;
	BYTE* x = NULL;
	//cout << "frame thread:" << std::this_thread::get_id() << endl;
	WaitForSingleObject(g_picevent, INFINITE);
	//数据出列
	if (0<m_pPicList.size())
	{
		try {
			x = m_pPicList.front();
			m_pPicList.pop_front();
			t_buffer = new BYTE[IMGSIZE];
			if (x != NULL)
			{
				memcpy(t_buffer, x, IMGSIZE);
				delete[]x;
				x = NULL;
			}
		}
		catch (const std::exception&err)
		{
			std::cout << err.what() << std::endl;
			return NULL;
		}
	}
//	SetEvent(g_videoevent);
	return t_buffer;
}

