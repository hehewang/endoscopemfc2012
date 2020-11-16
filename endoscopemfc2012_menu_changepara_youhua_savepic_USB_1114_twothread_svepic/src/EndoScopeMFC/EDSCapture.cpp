// DWCapture.cpp : 实现文件
// CEDSCapture类用于在directshow下实现简单的摄像头视频显示、暂停、单帧采样功能
// CEDSGrabberCB 是一个 ISampleGrabberCB 扩展类，重载了视频流的回调函数，用于保存图像。
// CEDSCapture类中调用CEDSGrabberCB成员实现单帧捕获。
// 该类还封装了directshow 接口基本的初始化，运行和关闭操作。
// --------Jasper------ 2007.01.11
// 加入CUDA函数与代码。初步实现了使用CUDA技术处理回调图像数据。 ---2013.04.15


#include "pch.h"
#include "EDSCapture.h"

#include "testfuction.h"
#include<assert.h>



BOOL g_bOneShot = FALSE;	// 此变量用来标记回调函数是否拍照
BOOL g_bAdjust1 = FALSE;	// 此变量用来标记回调函数是否做视频增强
#define WM_GRAPHNOTIFY  WM_APP+100
// 此变量标记当前进行的视频操作，0为无操作，1为字幕，2为偏红，3为偏绿，
// 4为偏蓝，5为降噪，6为锐化，7为存图，8为箭头指示
int g_nprocess = 0;		




// CEDSCapture

IMPLEMENT_DYNAMIC(CEDSCapture, CWnd)
CEDSCapture::CEDSCapture()
{
	m_hApp = NULL;
	m_dwGraphRegister = 0;	
	m_pVW = NULL;
	m_pMC = NULL;
	m_pME = NULL;
	m_pGraph = NULL;
	m_pCapture = NULL;
	m_pGrabber = NULL;
	
	m_psCurrent = Stopped;
	//g_bOneShot = FALSE;
	g_nprocess = 0;

}

CEDSCapture::~CEDSCapture()
{
	//CloseInterfaces();	// 关闭接口
	//CoUninitialize();	// 结束COM

	//g_CB.m_pBufferList.clear();
}

BEGIN_MESSAGE_MAP(CEDSCapture, CWnd)
END_MESSAGE_MAP()


// 对类做一些初始化工作
HRESULT CEDSCapture::Init(int iDeviceID, HWND hWnd)
{
	HRESULT hr;
    IBaseFilter *pSrcFilter=NULL;
	
	m_hApp = hWnd;
    // Get DirectShow interfaces
    hr = GetInterfaces();
    if (FAILED(hr))
    {
        Msg(TEXT("Failed to get video interfaces!  hr=0x%x"), hr);
        return hr;
    }

    // Attach the filter graph to the capture graph
    hr = m_pCapture->SetFiltergraph(m_pGraph);
    if (FAILED(hr))
    {
        Msg(TEXT("Failed to set capture filter graph!  hr=0x%x"), hr);
        return hr;
    }

    // Use the system device enumerator and class enumerator to find
    // a video capture/preview device, such as a desktop USB video camera.
	// 使用设备和类计数器寻找视频捕捉设备
    hr = FindCaptureDevice(&pSrcFilter);
    if (FAILED(hr))
    {
        // Don't display a message because FindCaptureDevice will handle it
        return hr;
    }
   
    // Add Capture filter to our graph.
    hr = m_pGraph->AddFilter(pSrcFilter, L"视频捕获");
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n") 
            TEXT("If you have a working video capture device, please make sure\r\n")
            TEXT("that it is connected and is not being used by another application.\r\n\r\n")
            TEXT("The sample will now close."), hr);
        pSrcFilter->Release();
        return hr;
    }

	// 创建一个接口
	hr = m_pGrabber.CoCreateInstance( CLSID_SampleGrabber );
	if( !m_pGrabber ){
		AfxMessageBox(_T("Fail to create SampleGrabber, maybe qedit.dll is not registered?"));
		return hr;
	}
	CComQIPtr< IBaseFilter, &IID_IBaseFilter > pGrabBase( m_pGrabber );

	//CMediaType VideoType;
	//设置视频格式
	AM_MEDIA_TYPE mt; 
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	hr = m_pGrabber->SetMediaType(&mt);

	if( FAILED( hr ) ){
		AfxMessageBox(_T("Fail to set media type!"));
		return hr;
	}
	hr = m_pGraph->AddFilter( pGrabBase, L"Grabber" );
	if( FAILED( hr ) ){
		AfxMessageBox(_T("Fail to put sample grabber in graph"));
		return hr;
	}

	// Render the preview pin on the video capture filter
	// Use this instead of m_pGraph->RenderFile
	IBaseFilter* pNull = NULL;
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)& pNull);
	hr = m_pGraph->AddFilter(pNull, L"NullRender");
	hr = m_pCapture->RenderStream( &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
                                   pSrcFilter, pGrabBase, pNull);	
	
	hr = m_pGrabber->GetConnectedMediaType( &mt );
	if ( FAILED( hr) ){
		AfxMessageBox(_T("Failt to read the connected media type"));
		return hr;
	}

	VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*) mt.pbFormat;
	g_CB.lWidth = vih->bmiHeader.biWidth;		// 设置图像宽度
	g_CB.lHeight = vih->bmiHeader.biHeight;		// 设置图像高度

	//g_CB.SetIplImage();	// 创建Ipl对象
	FreeMediaType(mt);
	hr = m_pGrabber->SetBufferSamples(TRUE);	// 初始化Sample指针
	hr = m_pGrabber->SetOneShot( FALSE );		// fliter接收一帧sample以后不停

	hr = m_pGrabber->SetCallback( &g_CB, 1 );	// 设置启动回调

    // Now that the filter has been added to the graph and we have
    // rendered its stream, we can release this reference to the filter.
    pSrcFilter->Release();

	// Set video window style and position
	// 设定窗口风格、位置大小
    //hr = SetupVideoWindow();//设置了预览窗口
   /* if (FAILED(hr))
    {
        Msg(TEXT("Couldn't initialize video window!  hr=0x%x"), hr);
		return hr;
	}*/

#ifdef REGISTER_FILTERGRAPH
    // Add our graph to the running object table, which will allow
    // the GraphEdit application to "spy" on our graph
    hr = AddGraphToRot(m_pGraph, &m_dwGraphRegister);
    if (FAILED(hr))
    {
        Msg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
        m_dwGraphRegister = 0;
    }
#endif

    // 开始启动视频预览 Start previewing video data
    hr = m_pMC->Run();
    if (FAILED(hr)) {
        Msg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
        return hr;
    }	

    // 记下当前状态
    m_psCurrent = Running;
    return S_OK;
}

// 暂停视频捕捉
void CEDSCapture::Pause()
{
	if( m_pMC ) {
		OAFilterState t_State;
		HRESULT hr = m_pMC->GetState(10, &t_State);
		if (hr == S_OK || hr == VFW_S_CANT_CUE) {
			hr = E_FAIL;
			switch (t_State)
			{
			case State_Running:
				hr = m_pMC->Pause();
				break;
			case State_Paused:
				hr = S_FALSE;
				break;
			case State_Stopped:
				hr = S_FALSE;
			default:
				break;
			}
		}
	}
}

// 继续视频捕捉
void CEDSCapture::Run()
{
	if( m_pMC ) {
		OAFilterState t_State;
		HRESULT hr = m_pMC->GetState(10, &t_State);
		if (hr == S_OK || hr == VFW_S_CANT_CUE) {
			hr = E_FAIL;
			switch (t_State)
			{
			case State_Running:
				break;
			case State_Paused:
				hr = m_pMC->Run();
				break;
			case State_Stopped:
				hr = m_pMC->Run();
				break;
			default:
				break;
			}
		}
	}
}
void CEDSCapture::Stop()
{
	if (m_pMC) {
		OAFilterState t_State;
		HRESULT hr = m_pMC->GetState(10, &t_State);
		if (hr == S_OK || hr == VFW_S_CANT_CUE) {
			hr = E_FAIL;
			switch (t_State)
			{
			case State_Running:
				hr = m_pMC->Stop();
				break;
			case State_Paused:
				hr = m_pMC->Stop();
				break;
			case State_Stopped:
				break;
			default:
				break;
			}
		}
	}
}

// 视频增强处理
void CEDSCapture::VideoAdjust( int ntype)
{
	switch( ntype ) {
	case 1:
		g_nprocess = 2;
		break;
	case 2:
		g_nprocess = 3;
		break;
	case 3:
		g_nprocess = 4;
		break;
	case 4:
		g_nprocess = 9;
		break;
	default:
		;
	}
}

// 视频增强处理
void CEDSCapture::DeNoise()
{
	g_nprocess = 5;		
}

// 视频锐化处理
void CEDSCapture::Sharped()
{
	g_nprocess = 6;		
}




void CEDSCapture::Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    _vsntprintf_s(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    MessageBox( szBuffer, TEXT("PlayCap Message"), MB_OK | MB_ICONERROR);
}

// 设置视频窗口
HRESULT CEDSCapture::SetupVideoWindow(void)
{
    HRESULT hr;

    // Set the video window to be a child of the main window
    hr = m_pVW->put_Owner((OAHWND)m_hApp);
    if (FAILED(hr))
        return hr;
	
	HWND hWnd1;
	//m_pVW->get_Owner( (OAHWND *)&hWnd1);
	hWnd1 = ::GetParent(m_hApp);
	m_pVW->put_MessageDrain( (OAHWND)hWnd1 );
    
    // Set video window style
    hr = m_pVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
    if (FAILED(hr))
        return hr;

    // Use helper function to position video window in client rect 
    // of main application window
    ResizeVideoWindow();

    // Make the video window visible, now that it is properly positioned
    hr = m_pVW->put_Visible(OATRUE);
    if (FAILED(hr))
        return hr;

    return hr;
}

// 调整视频窗口大小
void CEDSCapture::ResizeVideoWindow(void)
{
    // Resize the video preview window to match owner window size
    if ( m_pVW)    {
        RECT rc;        
        // Make the preview video fill our window
		::GetClientRect(m_hApp, &rc);
        m_pVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
		m_fscale_x = (rc.right + 1)*1.0f / g_CB.lWidth;
		m_fscale_y = (rc.bottom + 1)*1.0f / g_CB.lHeight;
    }
}

// 寻找捕捉设备
HRESULT CEDSCapture::FindCaptureDevice(IBaseFilter ** ppSrcFilter)
{
    HRESULT hr;
    IBaseFilter * pSrc = NULL;
    CComPtr <IMoniker> pMoniker =NULL;
    ULONG cFetched;

    if (!ppSrcFilter)
        return E_POINTER;
   
    // Create the system device enumerator
    CComPtr <ICreateDevEnum> pDevEnum =NULL;

    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                           IID_ICreateDevEnum, (void **) &pDevEnum);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create system enumerator!  hr=0x%x"), hr);
        return hr;
    }

    // Create an enumerator for the video capture devices
    CComPtr <IEnumMoniker> pClassEnum = NULL;

    hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr))
    {
        Msg(TEXT("Couldn't create class enumerator!  hr=0x%x"), hr);
        return hr;
    }

    // If there are no enumerators for the requested type, then 
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    if (pClassEnum == NULL)
    {
        MessageBox( TEXT("No video capture device was detected.\r\n\r\n")
                   TEXT("This sample requires a video capture device, such as a USB WebCam,\r\n")
                   TEXT("to be installed and working properly.  The sample will now close."),
                   TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
        return E_FAIL;
    }

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.
    if (S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched)))
    {
		IPropertyBag* t_pPropBag = NULL;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)& t_pPropBag);
		if (FAILED(hr)) {
			return hr;
			
		}
        // Bind Moniker to a filter object
        hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
        if (FAILED(hr))
        {
            Msg(TEXT("Couldn't bind moniker to filter object!  hr=0x%x"), hr);
            return hr;
        }

		VARIANT t_strName;
		VariantInit(&t_strName);
		hr = t_pPropBag->Read(L"FriendlyName", &t_strName, NULL);
		if (FAILED(hr)) {
			return hr;
		}
    }
    //else
    //{
   //     Msg(TEXT("Unable to access video capture device!"));   
    //    return E_FAIL;
    //}

    // Copy the found filter pointer to the output parameter.
    // Do NOT Release() the reference, since it will still be used
    // by the calling function.
    *ppSrcFilter = pSrc;

    return hr;
}

// 创建并获取DW接口
HRESULT CEDSCapture::GetInterfaces(void)
{
    HRESULT hr;

    // Create the filter graph
    hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                           IID_IGraphBuilder, (void **) &m_pGraph);
    if (FAILED(hr))
        return hr;

    // Create the capture graph builder
    hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
                           IID_ICaptureGraphBuilder2, (void **) &m_pCapture);
    if (FAILED(hr))
        return hr;
    
    // Obtain interfaces for media control and Video Window
    hr = m_pGraph->QueryInterface( IID_IMediaControl,(LPVOID *)&m_pMC );
    if (FAILED(hr))
        return hr;

    hr = m_pGraph->QueryInterface( IID_IVideoWindow, (LPVOID *)&m_pVW );
    if (FAILED(hr))
        return hr;

    hr = m_pGraph->QueryInterface( IID_IMediaEvent, (LPVOID *)&m_pME );
    if (FAILED(hr))
        return hr;

    // Set the window handle used to process graph events
    hr = m_pME->SetNotifyWindow((OAHWND)m_hApp, WM_GRAPHNOTIFY, 0);

    return hr;
}

// 关闭接口
void CEDSCapture::CloseInterfaces(void)
{
    // Stop previewing data
    if (m_pMC)
        m_pMC->StopWhenReady();

    m_psCurrent = Stopped;

    // Stop receiving events
    if (m_pME)
        m_pME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

    // Relinquish ownership (IMPORTANT!) of the video window.
    // Failing to call put_Owner can lead to assert failures within
    // the video renderer, as it still assumes that it has a valid
    // parent window.
    if(m_pVW)
    {
        m_pVW->put_Visible(OAFALSE);
        m_pVW->put_Owner(NULL);
    }

#ifdef REGISTER_FILTERGRAPH
    // Remove filter graph from the running object table   
    /*if (m_dwGraphRegister)
        RemoveGraphFromRot(m_dwGraphRegister);*/
#endif

    // 释放 DirectShow 接口
    SAFE_RELEASE(m_pMC);
    SAFE_RELEASE(m_pME);
    SAFE_RELEASE(m_pVW);
    SAFE_RELEASE(m_pGraph);
    SAFE_RELEASE(m_pCapture);
}


void CEDSCapture::FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0) {
		CoTaskMemFree((PVOID)mt.pbFormat);
		// Strictly unnecessary but tidier
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL) {
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}


//void CEDSCapture::SetupLamdaRGB()
//{
//	//CalculateLamda_RB(g_CB.m_pBuffer,g_CB.lWidth,g_CB.lHeight,&m_dlamdargb);
//	g_CB.m_badjwhite = true;
//}
/********************************************
*函数接口，用于调取一帧buffer，涉及多线程调用这个接口，所以要做同步处理
*
*
********************************************/
//BYTE* CEDSCapture::GetFrame()
//{
//	BYTE* t_buffer = NULL;
//	BYTE* x = NULL;
//	//cout << "frame thread:" << std::this_thread::get_id() << endl;
//	WaitForSingleObject(g_videoevent, INFINITE);
//	//数据出列
//	if (0<g_CB.m_pTaskList.size())
//	{
//		try {
//			x = g_CB.m_pTaskList.front();
//			g_CB.m_pTaskList.pop_front();
//			t_buffer = new BYTE[IMGSIZE];
//			if (x != NULL)
//			{
//				memcpy(t_buffer, x, IMGSIZE);
//				delete[]x;
//				x = NULL;
//			}
//		}
//		catch (const std::exception&err)
//		{
//			std::cout << err.what() << std::endl;
//			return NULL;
//		}
//	}
//	SetEvent(g_videoevent);
//	return t_buffer;
//}
//BYTE* CEDSCapture::GetPicture()
//{
//	BYTE* t_buffer = NULL;
//	BYTE* x = NULL;
//	//cout << "frame thread:" << std::this_thread::get_id() << endl;
//	WaitForSingleObject(g_picevent, INFINITE);
//	//数据出列
//	if (0<g_CB.m_pPicList.size())
//	{
//		try {
//			x = g_CB.m_pPicList.front();
//			g_CB.m_pPicList.pop_front();
//			t_buffer = new BYTE[IMGSIZE];
//			if (x != NULL)
//			{
//				memcpy(t_buffer, x, IMGSIZE);
//				delete[]x;
//				x = NULL;
//			}
//		}
//		catch (const std::exception&err)
//		{
//			std::cout << err.what() << std::endl;
//			return NULL;
//		}
//	}
////	SetEvent(g_videoevent);
//	return t_buffer;
//}

//void CEDSCapture::StartWriteVideo()
//{
//	g_CB.m_bwritevideo = true;
//}
//void CEDSCapture::StopWriteVideo()
//{
//	g_CB.m_bwritevideo = false;
//}
//void CEDSCapture::StartWritePic()
//{
//	g_CB.m_bwritepicture = true;
//}
//void CEDSCapture::StopWritePic()
//{
//	g_CB.m_bwritepicture = false;
//}


//void CEDSCapture::SetZoom(double flag)
//{
//	WaitForSingleObject(g_resizeevent, INFINITE);
//	g_CB.m_dZoom=flag;
//	g_CB.roi_w = IMGWIDTH*flag;
//	g_CB.roi_h = IMGHEIGHT*flag;
//	g_CB.roi_c=(g_CB.roi_w-IMGWIDTH)/2;
//	g_CB.roi_r=(g_CB.roi_h-IMGHEIGHT)/2;
//	SetEvent(g_resizeevent);
//}
// CEDSCapture 消息处理程序

//bool CEDSCapture::GetStateWhiteBalance()
//{
//	return g_CB.m_badjwhite;
//}