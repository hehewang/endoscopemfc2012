#pragma once
#include "dshow.h"
#include "D3d9.h"
#include "Vmr9.h"
#include "mw_cc708_dshow_decoder_uid.h"
#include "mw_iidecoder_set.h"
#include <vector>
using namespace std;
#pragma comment(lib,"strmiids.lib")
class MWCCGraphBuilder
{
public:
	MWCCGraphBuilder();
	~MWCCGraphBuilder();
public:
	IGraphBuilder *m_pGraphBuilder;
	IMediaControl *m_pMediaControl;
	IMediaEvent *m_pMediaEvent;

	IBaseFilter *m_pVMRRender9;
	IVMRFilterConfig9 *m_pVMR9Config;
	IVMRFilterConfig *m_pConfig;
	IVMRWindowlessControl *m_pWindowlessControl;
	IVMRWindowlessControl9 *m_pVMR9WindowlessControl;
	IPin *m_pVMR9PinPreview;
	IPin *m_pVMR9PinCC;

	vector<IBaseFilter *> m_vecDevices;
	vector<IPropertyBag *> m_vecPropBag;
	vector<wstring> m_vec_DeviceName;
	vector<string> m_vec_strDeviceName;
	IBaseFilter *m_pSDIIn;
	IPin *m_pDevPinPreview;
	IPin *m_pDevPinCC;

	IBaseFilter *m_pCCDecoder;
	IIDecoderSet *m_pSet;
	IPin *m_pCCPinIn;
	IPin *m_pCCPinOut;

	bool m_bWithCC;
	bool m_bHasCC;
	bool m_bConnect;
	bool m_bConnectCC;

public:
	HRESULT InitVMR9(HWND hwndApp);
	HRESULT EnumDevices();
	HRESULT CreateCCDecoder();

	HRESULT ConnectFilters(int i);
	void DisconnectFilters();
	HRESULT ConnectFiltersWithCC();
	void DisconectFiltersWithCC();
};

