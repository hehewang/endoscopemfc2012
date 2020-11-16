/*
*作者：李健
*日期：2020年2月
*描述：此类是源自于美乐威采集卡官方示例程序，主要用于采集卡
*	   采集数据，使用的方法是directshow,使用了VRM9进行显示。
*	   使用时，直接初始化对象使用，类中封装了暂停、播放，可
*	   可以参考EndoScopeMFCDlg.cpp中的应用。
*/

#include "pch.h"
#include "MWCCGraphBuilder.h"
#include "stdio.h"

//构造graph
MWCCGraphBuilder::MWCCGraphBuilder()
{
	m_pGraphBuilder = NULL;
	m_pMediaControl = NULL;
	m_pMediaEvent = NULL;

	m_pVMRRender9 = NULL;
	m_pConfig = NULL;

	m_pCCDecoder = NULL;

	m_bConnect = false;
	m_bConnectCC = false;

	HRESULT hr;
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)& m_pGraphBuilder);
	if (FAILED(hr)) {
		printf("filter graph create failed\n");
		m_pGraphBuilder = NULL;
		return;
	}
	hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void**)& m_pMediaControl);//查询控制接口
	if (FAILED(hr)) {
		m_pGraphBuilder->Release();
		printf("can not get media control\n");
		m_pGraphBuilder = NULL;
		m_pMediaControl = NULL;
		return;
	}
	hr = m_pGraphBuilder->QueryInterface(IID_IMediaEvent, (void**)& m_pMediaEvent);//查询事件接口
	if (FAILED(hr)) {
		m_pMediaControl->Release();
		m_pGraphBuilder->Release();
		printf("can not get media event\n");
		m_pGraphBuilder = NULL;
		m_pMediaControl = NULL;
		m_pMediaControl = NULL;
		return;
	}
}

MWCCGraphBuilder::~MWCCGraphBuilder()
{
	if (m_pCCPinIn != NULL)
		m_pCCPinIn->Release();
	if (m_pCCPinOut != NULL)
		m_pCCPinOut->Release();
	if (m_pSet != NULL)
		m_pSet->Release();
	if (m_pCCDecoder != NULL)
		m_pCCDecoder->Release();

	if (m_pVMR9WindowlessControl != NULL)
		m_pVMR9WindowlessControl->Release();
	if (m_pVMR9Config != NULL)
		m_pVMR9Config->Release();
	if (m_pVMRRender9 != NULL)
		m_pVMRRender9->Release();

	while (!m_vecDevices.empty()) {
		m_vecDevices.back()->Release();
		m_vecDevices.pop_back();
		m_vecPropBag.back()->Release();
		m_vecPropBag.pop_back();
	}

	if (m_pMediaControl != NULL)
		m_pMediaControl->Release();
	if (m_pMediaEvent != NULL)
		m_pMediaEvent->Release();
	ULONG t;
	if (m_pGraphBuilder != NULL)
		t = m_pGraphBuilder->Release();
	int i = 0;
	int j = 0;
}


//初始化渲染器VMR9
HRESULT MWCCGraphBuilder::InitVMR9(HWND hwndApp)
{
	HRESULT hr = NO_ERROR;
	if (m_pGraphBuilder == NULL) {
		hr = E_POINTER;
		return hr;
	}
	m_pVMRRender9 = NULL;
	hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)& m_pVMRRender9);
	if (FAILED(hr)) {
		printf("create VMR9 failed\n");
		return hr;
	}

	hr = m_pVMRRender9->QueryInterface(IID_IVMRFilterConfig9, (void**)& m_pVMR9Config);
	if (SUCCEEDED(hr)) {
		hr = m_pVMR9Config->SetRenderingMode(VMR9Mode_Windowless);
	}
	hr = m_pVMRRender9->QueryInterface(IID_IVMRWindowlessControl9, (void**)& m_pVMR9WindowlessControl);
	if (SUCCEEDED(hr)) {
		hr = m_pVMR9WindowlessControl->SetVideoClippingWindow(hwndApp);
		RECT* clientRect = new RECT;
		::GetClientRect(hwndApp, clientRect);
		hr = m_pVMR9WindowlessControl->SetVideoPosition(NULL, clientRect);
		delete clientRect;
	}
	return hr;
}
//枚举设备
HRESULT MWCCGraphBuilder::EnumDevices()
{
	HRESULT hr;
	m_vecDevices.clear();//清空设备向量
	//创建枚举对象
	ICreateDevEnum* t_pEunm = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)& t_pEunm);
	if (FAILED(hr))
		return hr;
	//创建枚举指示器
	IEnumMoniker* t_pEnumCat = NULL;
	hr = t_pEunm->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &t_pEnumCat, 0);
	if (FAILED(hr)) {
		t_pEunm->Release();
		return hr;
	}
	//如果没有则退出
	if (t_pEnumCat == NULL) {
		return -1;
	}

	IMoniker* t_pMonker = NULL;
	ULONG t_Feched;
	//循环查询设备
	while (t_pEnumCat->Next(1, &t_pMonker, &t_Feched) == S_OK) {
		IPropertyBag* t_pPropBag = NULL;
		hr = t_pMonker->BindToStorage(0, 0, IID_IPropertyBag, (void**)& t_pPropBag);
		if (FAILED(hr)) {
			continue;
		}
		m_vecPropBag.push_back(t_pPropBag);//存储
		IBaseFilter* t_pDev = NULL;
		hr = t_pMonker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)& t_pDev);//绑定设备
		if (FAILED(hr)) {
			m_vecPropBag.back()->Release();
			m_vecPropBag.pop_back();
			continue;
		}
		m_vecDevices.push_back(t_pDev);//存储设备

	}
	//循环加载设备名称存入成员变量vector中
	for (int i = 0; i < m_vecDevices.size(); i++) {
		IBaseFilter* t_pDev = m_vecDevices.at(i);
		IPropertyBag* t_pPropBag = m_vecPropBag.at(i);
		VARIANT t_strName;
		VariantInit(&t_strName);
		hr = t_pPropBag->Read(L"FriendlyName", &t_strName, NULL);
		if (FAILED(hr)) {
			continue;
		}
		wstring szName;
		szName.append((wchar_t*)t_strName.bstrVal);
		m_vec_DeviceName.push_back(szName);
		unsigned char strName[128];
		memset(strName, 0, 128);
		string s;
		for (int j = 0; j < szName.size(); j++)
		{
			strName[j] = ((unsigned char)szName.at(j));
			s.push_back(strName[j]);
		}
		m_vec_strDeviceName.push_back(s);

		VariantClear(&t_strName);
	}
	return hr;
}
//创建CC解码器，这个解码器应该是MW自己写的filter
HRESULT MWCCGraphBuilder::CreateCCDecoder()
{
	HRESULT hr;
	hr = CoCreateInstance(CLSID_MWCC708Decoder, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)& m_pCCDecoder);
	if (FAILED(hr)) {
		m_pCCDecoder = NULL;
	}
	IIDecoderSet* t_pSet = NULL;
	hr = m_pCCDecoder->QueryInterface(IID_DecoderSet, (void**)& t_pSet);
	m_pSet = t_pSet;//一个设置接口，解码器的设置接口
	t_pSet->SetFtcColor(true, 0, 255, 255, 255);
	m_pCCPinIn = NULL;
	m_pCCPinOut = NULL;
	return hr;
}
//连接filters，传入参数i：
HRESULT MWCCGraphBuilder::ConnectFilters(int i)
{
	i--;
	m_pDevPinCC = NULL;
	if (m_bConnect == true)
		return E_FAIL;

	if (!(i >= 0 && i < m_vecDevices.size())) {
		return E_FAIL;
	}
	m_pSDIIn = m_vecDevices.at(i);
	OAFilterState t_State;//获取控制接口的状态，用于判断当前的graph是否运行
	HRESULT hr = m_pMediaControl->GetState(10, &t_State);//停止graph运行
	if (hr == S_OK || hr == VFW_S_CANT_CUE) {
		switch (t_State)
		{
		case State_Running:
			m_pMediaControl->Stop();
			break;
		case State_Paused:
			m_pMediaControl->Stop();
			break;
		case State_Stopped:
			break;
		default:
			break;
		}
	}
	if (m_pGraphBuilder == NULL || m_pSDIIn == NULL || m_pVMRRender9 == NULL)
		return E_FAIL;
	//判断是否完成初始化builder、device、VMR渲染器，并加入builder
	m_pGraphBuilder->AddFilter(m_pSDIIn, L"SDI");
	m_pGraphBuilder->AddFilter(m_pVMRRender9, L"VMR9");
	//接口pin查询连接
	IEnumPins * t_pEnumPins = NULL;
	hr = m_pSDIIn->EnumPins(&t_pEnumPins);
	if (FAILED(hr)) {
		return hr;
	}
	IPin* t_Pin = NULL;
	IPin* t_Pin_CC = NULL;
	IPin* t_Pin_Preview = NULL;
	ULONG t_Fetched;
	while (t_pEnumPins->Next(1, &t_Pin, &t_Fetched) == S_OK)
	{
		PIN_INFO t_info;
		t_Pin->QueryPinInfo(&t_info);
		const wchar_t name[3] = { 25429,33719,0 };//capture in chinese
		if ((wcscmp(t_info.achName, name) == 0) || (wcscmp(t_info.achName, L"Capture") == 0)) {
			m_pDevPinPreview = t_Pin;//如果是capture接口
			t_Pin_Preview = t_Pin;
		}
		else if (wcscmp(t_info.achName, L"CC") == 0) {
			m_pDevPinCC = t_Pin;//如果是CC接口
			t_Pin_CC = t_Pin;
			m_bHasCC = true;//判断有CC接口，这个CC是什么
		}
		else {
			t_Pin->Release();
			t_Pin = NULL;
		}
	}
	t_pEnumPins->Release();
	t_pEnumPins = NULL;

	IEnumPins* t_pEnumVMRPins = NULL;
	hr = m_pVMRRender9->EnumPins(&t_pEnumVMRPins);
	if (FAILED(hr)) {
		if (t_Pin != NULL) {
			t_Pin->Release();
			t_Pin = NULL;
		}
		return hr;
	}
	IPin* t_pVMRPin = NULL;
	ULONG t_VMRFetched;
	while (t_pEnumVMRPins->Next(1, &t_pVMRPin, &t_VMRFetched) == S_OK) {
		break;
	}
	m_pVMR9PinPreview = t_pVMRPin;
	t_pEnumVMRPins->Release();
	t_pEnumVMRPins = NULL;

	if (t_pVMRPin != NULL && t_Pin_Preview != NULL) {
		hr = m_pGraphBuilder->Connect(t_Pin_Preview, t_pVMRPin);//将device的pin，连接到了vmr的引脚上
		if (SUCCEEDED(hr)) {
			m_bConnect = true;
		}
	}
	return hr;
}

void MWCCGraphBuilder::DisconnectFilters()
{
	if (m_bConnect == false)
		return;

	if (m_pGraphBuilder == NULL || m_pSDIIn == NULL || m_pVMRRender9 == NULL)
		return;
	if (m_pMediaControl == NULL)
		return;
	OAFilterState t_State;
	HRESULT hr = m_pMediaControl->GetState(10, &t_State);//必须停止graph才能进行链路修改
	if (hr == S_OK || hr == VFW_S_CANT_CUE) {
		switch (t_State)
		{
		case State_Running:
			m_pMediaControl->Stop();
			break;
		case State_Paused:
			m_pMediaControl->Stop();
			break;
		case State_Stopped:
			break;
		default:
			break;
		}
	}
	if (m_pDevPinPreview != NULL) {
		//m_pGraphBuilder->Disconnect(m_pDevPinPreview);
	}
	m_pGraphBuilder->RemoveFilter(m_pVMRRender9);
	m_pGraphBuilder->RemoveFilter(m_pSDIIn);
	m_bConnect = false;

}
//带CC解码器的连接链路
HRESULT MWCCGraphBuilder::ConnectFiltersWithCC()
{
	if (m_bConnectCC == true)
		return E_FAIL;

	OAFilterState t_State;
	HRESULT hr = m_pMediaControl->GetState(10, &t_State);
	if (hr == S_OK || hr == VFW_S_CANT_CUE) {
		switch (t_State)
		{
		case State_Running:
			m_pMediaControl->Stop();
			break;
		case State_Paused:
			m_pMediaControl->Stop();
			break;
		case State_Stopped:
			break;
		default:
			break;
		}
	}

	if (m_bConnect == false)
		return E_FAIL;

	if (m_pCCDecoder == NULL)
		return E_FAIL;
	m_pGraphBuilder->AddFilter(m_pCCDecoder, L"CC708Decoder");
	if (m_pDevPinCC == NULL)
		return E_FAIL;

	IPin * t_pCCPin_In = NULL;
	IPin * t_pCCPin_Out = NULL;

	if (m_pCCDecoder != NULL) {
		IEnumPins* t_pEnumCCDecoderPins = NULL;
		hr = m_pCCDecoder->EnumPins(&t_pEnumCCDecoderPins);
		if (SUCCEEDED(hr)) {
			IPin* t_pCCPin = NULL;

			ULONG t_CCFetched;
			while (t_pEnumCCDecoderPins->Next(1, &t_pCCPin, &t_CCFetched) == S_OK) {
				PIN_INFO t_info;
				t_pCCPin->QueryPinInfo(&t_info);
				if (t_info.dir == PINDIR_INPUT) {
					if (NULL == m_pCCPinIn) {
						m_pCCPinIn = t_pCCPin;
						t_pCCPin_In = t_pCCPin;
					}
					else {
						t_pCCPin->Release();
					}
				}
				else {
					if (NULL == m_pCCPinOut) {
						m_pCCPinOut = t_pCCPin;
						t_pCCPin_Out = t_pCCPin;
					}
					else {
						t_pCCPin->Release();
					}
				}

				if (t_info.pFilter)
					t_info.pFilter->Release();

				if (NULL != m_pCCPinIn && NULL != m_pCCPinOut)
					break;
			}

			t_pEnumCCDecoderPins->Release();
		}
	}

	IEnumPins* t_pEnumVMRPins = NULL;
	hr = m_pVMRRender9->EnumPins(&t_pEnumVMRPins);
	if (FAILED(hr))
		return hr;
	IPin* t_pVMRPin2 = NULL;
	ULONG t_VMRFetched;
	while (t_pEnumVMRPins->Next(1, &t_pVMRPin2, &t_VMRFetched) == S_OK) {
		IPin* t_CPin = NULL;
		t_pVMRPin2->ConnectedTo(&t_CPin);
		if (t_CPin != NULL) {
			t_pVMRPin2 = NULL;
			continue;
		}
		else {
			break;
		}
		break;
	}
	m_pVMR9PinCC = t_pVMRPin2;
	t_pEnumVMRPins->Release();
	t_pEnumVMRPins = NULL;

	if (m_pVMR9PinCC == NULL)
		return E_FAIL;

	if (m_pDevPinCC != NULL && m_pCCPinIn != NULL && m_pCCPinOut != NULL && m_pVMR9PinCC != NULL) {
		hr = m_pGraphBuilder->Connect(m_pDevPinCC, m_pCCPinIn);
		if (SUCCEEDED(hr)) {
			hr = m_pGraphBuilder->Connect(m_pCCPinOut, m_pVMR9PinCC);
			if (SUCCEEDED(hr)) {
				m_bConnectCC = true;
			}
			else {
				m_bConnectCC = false;
				m_pGraphBuilder->Disconnect(m_pDevPinCC);
			}
		}
		else {
			m_bConnectCC = false;
		}

	}
	return hr;
}
void MWCCGraphBuilder::DisconectFiltersWithCC()
{
	if (m_bConnectCC == false)
		return;

	OAFilterState t_State;
	HRESULT hr = m_pMediaControl->GetState(10, &t_State);
	if (hr == S_OK || hr == VFW_S_CANT_CUE) {
		switch (t_State)
		{
		case State_Running:
			m_pMediaControl->Stop();
			break;
		case State_Paused:
			m_pMediaControl->Stop();
			break;
		case State_Stopped:
			break;
		default:
			break;
		}
	}
	if (m_bConnectCC == true) {
		if (m_pCCPinIn != NULL) {
			//hr=m_pGraphBuilder->Disconnect(m_pCCPinIn);
		}
		if (m_pCCPinOut != NULL) {
			//hr=m_pGraphBuilder->Disconnect(m_pCCPinOut);
		}
		m_bConnectCC = false;
		m_pGraphBuilder->RemoveFilter(m_pCCDecoder);
	}
}