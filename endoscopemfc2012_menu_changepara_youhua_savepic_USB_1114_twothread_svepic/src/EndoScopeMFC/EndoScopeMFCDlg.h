
// EndoScopeMFCDlg.h: 头文件
//

#pragma once
#include "MWCCGraphBuilder.h"
#include "EDSCapture.h"
#include <afxwin.h>
#include <list>

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
	CString msg1;
	CString msg2;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	BOOL SetInfo(CString s1, CString s2);
	BOOL bHaveDlg;
};

// CEndoScopeMFCDlg 对话框
class CEndoScopeMFCDlg : public CDialogEx
{
// 构造
public:
	CEndoScopeMFCDlg(CWnd* pParent = nullptr);	// 标准构造函数
	~CEndoScopeMFCDlg();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ENDOSCOPEMFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	MWCCGraphBuilder *m_pCCGrapgBuilder;
	// 摄像头窗口控件
	CStatic m_cam;
// 实现
protected:
	HICON m_hIcon;
	CRect m_Rect;
	void ChangeSize(CWnd *hwnd, int cx, int cy);
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButOk();
	afx_msg void OnBnClickedButRun();
	afx_msg void OnBnClickedButPause();
	afx_msg void OnBnClickedButStop();
	afx_msg LRESULT OnCtlData(WPARAM wParam, LPARAM lParam);//响应下位机指令
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
//	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void ShotPicWindow();
	void CaptureScreen();
	string GetSaveImgPath(int flag);
	void SaveCaptureImage(string filepath);
	void InitializeFormBoxLayout();
	void SetPicWidowShape();
	void ShowEndoscopeInfo(BYTE * pInfo, int nSize);//显示内镜信息
	void ImageSetting(unsigned char* pbuf);//图像设置
	void UpdateWhiteBalance();//更新白平衡参数
	void LightControler(unsigned char*pInfo);//光源控制器
	void RestoreFactorySetting();//恢复出厂设置
	void SetModeofMeterLight(unsigned char*pbuf);//测光模式
	void ChangeImageRB(unsigned char *pbuf,int flag);//变换RB值
	void EnterPatientInfo();//录入病人信息
	void SaveVideo();//录像
	void MoveDatatoUSBDevice();//移动数据到移动存储设备
	void SetZoom(double flag);//设置放大倍数
	void SetInitValue();//设置初始值

	void AdjWhiteBalance(unsigned char* src, int width, int height, unsigned char** dst);//计算
	void CalWhilteLamda(BYTE * buf);//计算白平衡系数
	void IniWhilteLamda();//初始化白平衡系数
	BYTE* GetFrame();
	BYTE* GetPicture();

private:
	int m_nPicWindow_width;
	int m_nPicWindow_height;
	int m_nPicWindow_left;
	int m_nPicWindow_top;
	HWND m_hPicWindow;
	CRect m_rPicRect;
	int m_nPicWindowShape;//窗口形状控制
	int m_nZoomIn;//放大倍数初始化1倍分别是0-1/1-1.2 / 2-1.5 / 3-2.0
	
	cv::Mat m_SaveImage;
	CString m_sdatapath;//数据存放地址
    bool m_bwritevideo;//判断是否写入视频
	bool m_bstartsave;//开始保存视频
	bool m_bwritepicture;//判断是否写入图片
	bool m_bstartpicture;//开始保存图片
	bool m_bshowpause;//显示暂停
	bool m_bsetuplamda;//判断是否更新白平衡系数
	bool m_bInputInfo;//判断是否输入信息
	bool m_bshowvideo;//判断是否显示
	bool m_brecvvideo;//判断是否接收
	CString m_strRB;
	CWinThread *pThreadVideo;//写入视频
	CWinThread *pThreadPic;//写入图片
	HANDLE m_hthreadprocess;
	HANDLE m_hthreadsavevideo;
	//键盘键值
	int m_f3key;
	int m_f4key;
	int m_f5key;
	int m_f6key;
	int m_f11key;
	int m_f2key;//调出嘉恒菜单
	int m_f7key;
	/*屏幕录制数据*/
	cv::VideoWriter m_vwriter;//写入视频
	int m_nScreenWidth ;//屏幕尺寸
	int m_nScreenHeight;//屏幕尺寸
	HWND m_hDeskWindow;//屏幕桌面句柄
	HDC m_hDesktopDC ;//屏幕DC
	HDC m_hBmpFileDC ;//写入bitmapDC
	HBITMAP m_hBitmap;//bit头
	int m_nPic;					//存储照片数
	/*视频采集类*/
	CEDSCapture m_dwcap;
	CAboutDlg dlgBoxInfo;
	//CEDSGrabberCB m_dgra;
	// 枚举视频运行状态

	double*m_lamda;//白平衡系数
	bool m_badjwhite;//是否调整白平衡
	bool m_bwhiteok;//白平衡是否矫正完成
	cv::Mat m_dst;
	cv::Mat m_src;//数据
	cv::Mat Picimg;//图像数据
	int roi_w ;
	int roi_h;
	int roi_c ;
	int roi_r ;
	double m_dZoom;    //放大倍数

	enum EDS_STATE {
		EDS_Init,
		EDS_Stopped,
		EDS_Paused,
		EDS_Running,
		EDS_Adjust1,
		EDS_Adjust2,
		EDS_Denoise,
		EDS_Sharped,
		EDS_Arrow
	};
	enum EDS_STATE m_eCurrent;

	std::list<BYTE*> m_pTaskList;//处理任务队列
	std::list<BYTE*> m_pPicList;//处理图片队列
	std::list<BYTE*> imglistforwhite;
private:
	void ShowCurrentDateTime();
	void InitializeThread();
	void InitializecvWindow();
	void InitializePLC();
	char* GetScreenDataGDI();
	static UINT ScreenCAPThreadProc(LPVOID lpParameter);
	static UINT SavePicThreadProc(LPVOID lpParameter);
	static UINT ProcessImage(LPVOID lpParameter);
	static UINT ShowVideoThreadFunc(LPVOID lpParameter);
	static UINT RecvVideoThreadFunc(LPVOID lpParameter);
	//导出数据
	void SaveToPeripherals();
	void InitParameter();
	void VideoZoom();
	//亮度
	void BrightnessAdd();
	void BrightnessSub();
	//对比度
	void ContrastAdd();
	void ContrastSub();
	//色调
	void HueAdd();
	void HueSub();
	//饱和度
	void SaturationAdd();
	void SaturationSub();
	//清晰度
	void SharpnessAdd();
	void SharpnessSub();
	void InitSharpness();
	//伽马
	void GammaAdd();
	void GammaSub();
	//增益
	void GainAdd();
	void GainSub();
	//曝光
	bool ExposeAdd();
	bool ExposeSub();
	//处理板菜单
	bool MenuStart();
	bool MenuStop();
	bool MenuUp();
	bool MenuDown();
	bool MenuLeft();
	bool MenuRight();
	bool MenuEnter();
};
