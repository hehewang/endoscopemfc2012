#include "pch.h"
#include "PLC.h"


CPLC *g_pPLC = NULL;

CPLC::CPLC(void)
{
	m_VideoComIO.SetDataHandler(this);
	m_LightComIO.SetDataHandler(this);
	m_bRegister = false;

	m_strSemState = _T("");
}


CPLC::~CPLC(void)
{
}

void CPLC::GetShowDlgPointer( CWnd *pShowDlg )
{
	ASSERT( NULL != pShowDlg );
	g_pShowDlg = pShowDlg;
}


bool CPLC::VideoInitialize( CString strCOM, int nBaudRate )
{
	return m_VideoComIO.Initialize( strCOM, nBaudRate );
}

bool CPLC::LightInitialize( CString strCOM, int nBaudRate )
{
	return m_LightComIO.Initialize( strCOM, nBaudRate );
}

bool CPLC::ProcessCardInitialize(CString strCOM, int nBaudRate)
{
	return m_ProcessCardComIO.Initialize(strCOM,nBaudRate);
}

void CPLC::ProcessData( unsigned char* pBuf, int size )
{
	if ( g_pShowDlg != NULL )
	{
		g_pShowDlg->SendMessage( WM_SERIALPORT_MSG, (WPARAM)pBuf, size );
	}
}
