
// EndoScopeMFC.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "EndoScopeMFC.h"
#include "EndoScopeMFCDlg.h"
#include<DbgHelp.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib,"Dbghelp.lib")
// CEndoScopeMFCApp

BEGIN_MESSAGE_MAP(CEndoScopeMFCApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CEndoScopeMFCApp 构造

CEndoScopeMFCApp::CEndoScopeMFCApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CEndoScopeMFCApp 对象

CEndoScopeMFCApp theApp;


/*函数作用：当软件崩溃时候将内存信息写入到dmp用于后期调试
*输入限制：此函数为回调函数，利用SetUnhandledExceptionFilter来设置回调
*/
LONG WINAPI ExceptionHandler(_In_ struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	//MessageBoxA(NULL, "Crash", "Error", MB_OK);
	//获取当前进程HANDLE
	HANDLE hproc = GetCurrentProcess();
	//获取当前进程ID
	int proId = GetCurrentProcessId();
	//以Windows API的方式创建一个文件句柄
	HANDLE hfile = CreateFile(_T("ErrorInformation.DMP"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile)
	{
		_MINIDUMP_EXCEPTION_INFORMATION minidumpInfo;
		minidumpInfo.ClientPointers = FALSE;
		minidumpInfo.ExceptionPointers = ExceptionInfo;
		minidumpInfo.ThreadId = GetCurrentThreadId();
		MiniDumpWriteDump(hproc, GetCurrentProcessId(), hfile, MiniDumpNormal, &minidumpInfo, NULL, NULL);
		CloseHandle(hfile);
	}
	return 0;
}


// CEndoScopeMFCApp 初始化

BOOL CEndoScopeMFCApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	ReadConfig();
	SetUnhandledExceptionFilter(ExceptionHandler);//设置崩溃的回调
	CoInitialize(NULL);

	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CEndoScopeMFCDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

int CEndoScopeMFCApp::ExitInstance()
{
	CoUninitialize();
	return CWinApp::ExitInstance();
}