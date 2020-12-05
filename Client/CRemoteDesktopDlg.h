#pragma once

class CRemoteDesktopDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRemoteDesktopDlg)

public:
	CRemoteDesktopDlg(DWORD dwTargetIp, CWnd* pParent = nullptr);
	virtual ~CRemoteDesktopDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REMOTE_DESKTOP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	DWORD m_dwTargetIp;
	SOCKET m_desktopSocket;
	SOCKET m_cmdSocket;

	bool Init();
	static DWORD WINAPI DesktopThreadProc(LPVOID lpThreadParameter);
	void ShowScreen(char* pBits, int nWidth, int nHeight);
	void SendMouseCmd(int nCmd, CPoint point);
	void SendKeyboardCmd(int nCmd, int nKey);

	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual void PostNcDestroy();
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
};
