#include "framework.h"
#include "Client.h"
#include "CRemoteFileDlg.h"
#include "afxdialogex.h"
#include "../Common/Common.h"
#include <iostream>
using namespace std;

IMPLEMENT_DYNAMIC(CRemoteFileDlg, CDialogEx)

CRemoteFileDlg::CRemoteFileDlg(DWORD dwTargetIp, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_REMOTE_FILE, pParent)
	, m_dwTargetIp(dwTargetIp)
	, m_localPathEdit(TRUE)
	, m_remotePathEdit(FALSE)
{
}

CRemoteFileDlg::~CRemoteFileDlg()
{
}

void CRemoteFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOCAL_FILE, m_localFileList);
	DDX_Control(pDX, IDC_LIST_REMOTE_FILE, m_remoteFileList);
	DDX_Control(pDX, IDC_EDIT_LOCAL_FILE_PATH, m_localPathEdit);
	DDX_Control(pDX, IDC_EDIT_REMOTE_FILE_PATH, m_remotePathEdit);
}

BEGIN_MESSAGE_MAP(CRemoteFileDlg, CDialogEx)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_LOCAL_FILE, &CRemoteFileDlg::OnDblclkListLocalFile)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_REMOTE_FILE, &CRemoteFileDlg::OnDblclkListRemoteFile)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_LOCAL_FILE, &CRemoteFileDlg::OnRclickListLocalFile)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_REMOTE_FILE, &CRemoteFileDlg::OnRclickListRemoteFile)
	ON_COMMAND(ID_UPLOAD, &CRemoteFileDlg::OnUpload)
	ON_COMMAND(ID_DOWNLOAD, &CRemoteFileDlg::OnDownload)
END_MESSAGE_MAP()

// 根据路径显示本地文件列表
BOOL CRemoteFileDlg::ShowLocalFileList(CString strFilePath)
{
	list<FileInfo> lt;
	if (!GetFileInfoList(strFilePath.GetBuffer(), lt))
	{
		return FALSE;
	}

	// 清空列表
	m_localFileList.DeleteAllItems();

	int i = 0;
	for (FileInfo info : lt)
	{
		m_localFileList.InsertItem(i, NULL);
		// 插入文件名
		m_localFileList.SetItemText(i, 0, info.strName.c_str());
		// 插入文件大小
		if (!info.strSize.empty())
		{
			m_localFileList.SetItemText(i, 1, info.strSize.c_str());
		}
		// 插入最后修改时间
		if (!info.strModifiedTime.empty())
		{
			m_localFileList.SetItemText(i, 2, info.strModifiedTime.c_str());
		}
		i++;
	}

	return TRUE;
}

// 根据路径显示远程文件列表
BOOL CRemoteFileDlg::ShowRemoteFileList(CString strFilePath)
{
	// 发送命令
	if (!SendInt(m_clientSocket, FILE_LIST))
	{
		return FALSE;
	}

	// 发送文件路径
	if (!SendData(m_clientSocket, strFilePath, strFilePath.GetLength() + 1))
	{
		return FALSE;
	}

	// 接收文件列表
	char* pBuf = NULL;
	int nLen = 0;
	if (!RecvData(m_clientSocket, pBuf, nLen) || -1 == nLen)
	{
		// 接收出错或者打开路径失败
		return FALSE;
	}
	unique_ptr<char> upBuf(pBuf);

	// 清空列表
	m_remoteFileList.DeleteAllItems();

	// 解析
	int i = 0;
	char* p = pBuf;
	while (p < pBuf + nLen)
	{
		m_remoteFileList.InsertItem(i, NULL);

		// 插入文件名
		m_remoteFileList.SetItemText(i, 0, p);
		p += strlen(p) + 1;
		// 插入文件大小
		m_remoteFileList.SetItemText(i, 1, p);
		p += strlen(p) + 1;
		// 插入最后修改时间
		m_remoteFileList.SetItemText(i, 2, p);
		p += strlen(p) + 1;

		i++;
	}

	return TRUE;
}

// 根据当前路径和文件名获得新的路径
CString CRemoteFileDlg::GetFilePath(CString strCurPath, CString strFileName)
{
	CString strNewPath;
	if ("" == strCurPath)
	{
		strNewPath = strFileName;
	}
	else if (".." == strFileName)
	{
		int n = strCurPath.ReverseFind('\\');
		strNewPath = strCurPath.Left(n);
	}
	else
	{
		strNewPath = strCurPath + "\\" + strFileName;
	}

	return strNewPath;
}

BOOL CRemoteFileDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_clientSocket = ConnectTargetHost(m_dwTargetIp);
	if (INVALID_SOCKET == m_clientSocket)
	{
		EndDialog(-1);
		return TRUE;
	}

	// 请求远程文件
	int nCmd = REMOTE_FILE;
	if (SOCKET_ERROR == send(m_clientSocket, (char*)&nCmd, sizeof(nCmd), 0))
	{
		PrintErrMsg("send");
		EndDialog(-1);
		return TRUE;
	}

	// 加载菜单资源
	m_menu.LoadMenu(IDR_MENU);

	// 设置列表控件样式
	m_localFileList.SetExtendedStyle(
		LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | m_localFileList.GetExtendedStyle());
	m_remoteFileList.SetExtendedStyle(
		LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | m_remoteFileList.GetExtendedStyle());

	// 添加列名
	char* cols[] = { "名称", "大小", "修改日期" };
	int nCnt = sizeof(cols) / sizeof(char*);
	for (int i = 0; i < nCnt; i++)
	{
		m_localFileList.InsertColumn(i, cols[i], 0, 200);
		m_remoteFileList.InsertColumn(i, cols[i], 0, 200);
	}

	// 显示本地盘符
	ShowLocalFileList("");

	// 显示远程盘符
	ShowRemoteFileList("");

	return TRUE;
}

// 双击打开本地文件
void CRemoteFileDlg::OnDblclkListLocalFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	// 获取文件名
	CString strFileName = m_localFileList.GetItemText(pNMItemActivate->iItem, 0);

	// 获取新路径
	CString strNewPath = GetFilePath(m_strCurLocalPath, strFileName);

	if (ShowLocalFileList(strNewPath))
	{
		// 若显示成功，则更新当前文件路径
		m_strCurLocalPath = strNewPath;
		// 同步到edit控件
		m_localPathEdit.SetWindowText(m_strCurLocalPath);
	}
}

// 双击打开远程文件
void CRemoteFileDlg::OnDblclkListRemoteFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	// 获取文件名
	CString strFileName = m_remoteFileList.GetItemText(pNMItemActivate->iItem, 0);

	// 获取新路径
	CString strNewPath = GetFilePath(m_strCurRemotePath, strFileName);

	if (ShowRemoteFileList(strNewPath))
	{
		// 若显示成功，则更新当前远程文件路径
		m_strCurRemotePath = strNewPath;
		// 同步到edit控件
		m_remotePathEdit.SetWindowText(m_strCurRemotePath);
	}
}


void CRemoteFileDlg::OnRclickListLocalFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	CMenu* pMenu = m_menu.GetSubMenu(0);

	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x	, pt.y, this);
}


void CRemoteFileDlg::OnRclickListRemoteFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	CMenu* pMenu = m_menu.GetSubMenu(1);

	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
}


void CRemoteFileDlg::OnUpload()
{
	int nRow = m_localFileList.GetSelectionMark();
	if (-1 == nRow)
	{
		AfxMessageBox("请选择文件");
	}
	CString strFileName = m_localFileList.GetItemText(nRow, 0);

	// 发送指令
	if (!SendInt(m_clientSocket, UPLOAD_FILE))
	{
		return;
	}
	// 发送文件名
	if (!SendData(m_clientSocket, strFileName, strFileName.GetLength() + 1))
	{
		return;
	}

	// 文件路径
	string strFilePath = m_strCurLocalPath + "\\" + strFileName;
	// 读取文件并发送
	SendFile(m_clientSocket, strFilePath);
}


void CRemoteFileDlg::OnDownload()
{
	int nRow = m_remoteFileList.GetSelectionMark();
	if (-1 == nRow)
	{
		AfxMessageBox("请选择文件");
	}
	CString strFileName = m_remoteFileList.GetItemText(nRow, 0);

	// 发送指令
	if (!SendInt(m_clientSocket, DOWNLOAD_FILE))
	{
		return;
	}
	// 发送文件名
	if (!SendData(m_clientSocket, strFileName, strFileName.GetLength() + 1))
	{
		return;
	}

	// 文件路径
	string strFilePath = m_strCurLocalPath + "\\" + strFileName;
	// 接收文件并保存
	RecvFile(m_clientSocket, strFilePath);
}
