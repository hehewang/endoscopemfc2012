// CSetPSW.cpp : 实现文件
//

#include "pch.h"
#include "EndoScopeMFC.h"
#include "CSetPSW.h"
#include "afxdialogex.h"
#include <ctype.h>


// CSetPSW 对话框

IMPLEMENT_DYNAMIC(CSetPSW, CDialogEx)

CSetPSW::CSetPSW(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSetPSW::IDD, pParent)
{

}

CSetPSW::~CSetPSW()
{
}

void CSetPSW::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSetPSW, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSetPSW::OnBnClickedOk)
END_MESSAGE_MAP()


// CSetPSW 消息处理程序


void CSetPSW::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CString sPath;
	GetModuleFileName(NULL, sPath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	sPath.ReleaseBuffer();

	int nPos;
	nPos = sPath.ReverseFind('\\');
	sPath = sPath.Left(nPos);

	CString ConfigPath = sPath + _T("\\config.ini");
	char cPSW[50];
	memset(cPSW, 0, 50 * sizeof(char));
	GetPrivateProfileString("LOGIN", "PASSWORD", "", cPSW, 50 * sizeof(char), ConfigPath);
	CString sOldPSW(cPSW);

	CString sOld,sNew,sNew2;
	GetDlgItemText(IDC_EDIT_OLDPSW, sOld);
	GetDlgItemText(IDC_EDIT_NEWPSW, sNew);
	GetDlgItemText(IDC_EDIT_NEWPSW2, sNew2);
	if ("" == sOld)
	{
		::MessageBox(NULL, "原密码不得为空！", "温馨提示", MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
		return;
	}
	if (sOldPSW != sOld)
	{
		::MessageBox(NULL, "原密码输入错误！", "温馨提示", MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
		return;
	}
	if ("" == sNew)
	{
		::MessageBox(NULL, "新密码不得为空！", "温馨提示", MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
		return;
	}
	if (sOld == sNew)
	{
		::MessageBox(NULL, "新密码不能和原密码相同！", "温馨提示", MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
		return;
	}
	//检查是否含有特殊字符
	char* pNewPSW = sNew.GetBuffer(0);
	if (noAnorD(pNewPSW))
	{
		::MessageBox(NULL, "新密码只能包含英文字母或数字（半角）！", "温馨提示", MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
		return;
	}

	if (sNew2 == sNew)
	{
		WritePrivateProfileString("LOGIN", "PASSWORD", sNew, ConfigPath);
	}
	else
	{
		::MessageBox(NULL, "两次新密码输入不一致！", "温馨提示", MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
		return;
	}
	CDialogEx::OnOK();
}
bool CSetPSW::noAnorD(const char* s)
{
	// TODO: 在此处添加实现代码.
	int r;
	r = 0;
	while(1)
	{
		if (*s == 0) break;
		if (!(isalpha(*s) | isdigit(*s)))//只能包含半角英文大写或小写字母或半角阿拉伯数字
			return true;
		s++;
	}
	return false;
}
