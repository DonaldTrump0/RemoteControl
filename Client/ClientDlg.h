#pragma once

class CClientDlg : public CDialogEx
{
public:
	CClientDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	SOCKET m_clientSocket;
	DWORD m_dwTargetIp;

	afx_msg void OnBnClickedButtonRemoteDesktop();
	afx_msg void OnBnClickedButtonRemoteCmd();
	afx_msg void OnBnClickedButtonRemoteFile();
};
