#pragma once


// CSetPSW 对话框

class CSetPSW : public CDialogEx
{
	DECLARE_DYNAMIC(CSetPSW)

public:
	CSetPSW(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSetPSW();

// 对话框数据
	enum { IDD = IDD_DLG_SetPSW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	bool noAnorD(const char* s);
};
