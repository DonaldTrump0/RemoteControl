#pragma once
#include "CFilePathEdit.h"

class CRemoteFileDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRemoteFileDlg)

public:
	CRemoteFileDlg(DWORD dwTargetIp, CWnd* pParent = nullptr);
	virtual ~CRemoteFileDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REMOTE_FILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	DWORD m_dwTargetIp;
	SOCKET m_clientSocket;
	CString m_strCurLocalPath;
	CString m_strCurRemotePath;
	CListCtrl m_localFileList;
	CListCtrl m_remoteFileList;
	CMenu m_menu;
	CFilePathEdit m_localPathEdit;
	CFilePathEdit m_remotePathEdit;

	BOOL ShowLocalFileList(CString strFilePath);
	BOOL ShowRemoteFileList(CString strFilePath);
	CString GetFilePath(CString strCurPath, CString strFileName);

	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkListLocalFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListRemoteFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpload();
	afx_msg void OnRclickListLocalFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickListRemoteFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownload();
};
