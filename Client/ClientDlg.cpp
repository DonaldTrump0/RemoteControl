#include "framework.h"
#include "Client.h"
#include "ClientDlg.h"
#include "CRemoteDesktopDlg.h"
#include "CRemoteFileDlg.h"
#include "../Common/Common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
	, m_dwTargetIp(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_TARGET, m_dwTargetIp);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_REMOTE_DESKTOP, &CClientDlg::OnBnClickedButtonRemoteDesktop)
	ON_BN_CLICKED(IDC_BUTTON_REMOTE_CMD, &CClientDlg::OnBnClickedButtonRemoteCmd)
	ON_BN_CLICKED(IDC_BUTTON_REMOTE_FILE, &CClientDlg::OnBnClickedButtonRemoteFile)
END_MESSAGE_MAP()

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//m_dwTargetIp = 0x7f000001;
	m_dwTargetIp = 0xc0a80107;
	UpdateData(FALSE);

	return TRUE;
}

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 远程桌面控制
void CClientDlg::OnBnClickedButtonRemoteDesktop()
{
	UpdateData(TRUE);

	CRemoteDesktopDlg* pRemoteDesktopDlg = new CRemoteDesktopDlg(m_dwTargetIp);
	if (!pRemoteDesktopDlg->Init())
	{
		delete pRemoteDesktopDlg;
		return;
	}

	pRemoteDesktopDlg->Create(IDD_DIALOG_REMOTE_DESKTOP);
	//pRemoteDesktopDlg->SetParent(GetDesktopWindow());
	//pRemoteDesktopDlg->ShowWindow(SW_SHOWNORMAL);
}

// 远程命令行控制
void CClientDlg::OnBnClickedButtonRemoteCmd()
{
	char pCmdLine[MAX_PATH] = { 0 };
	sprintf(pCmdLine, "../Debug/RemoteCmd.exe %u", m_dwTargetIp);

	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi = { 0 };

	if (!CreateProcess(0, pCmdLine, 0, 0, 0, 0, 0, 0, &si, &pi))
	{
		AfxMessageBox("远程命令行失败");
		PrintErrMsg("CreateProcess");
	}
}

// 远程文件
void CClientDlg::OnBnClickedButtonRemoteFile()
{
	UpdateData(TRUE);

	CRemoteFileDlg* pRemoteFileDlg = new CRemoteFileDlg(m_dwTargetIp);
	if (!pRemoteFileDlg->m_bInitSuccess)
	{
		delete pRemoteFileDlg;
		return;
	}

	pRemoteFileDlg->Create(IDD_DIALOG_REMOTE_FILE);
	pRemoteFileDlg->ShowWindow(SW_SHOWNORMAL);
}
