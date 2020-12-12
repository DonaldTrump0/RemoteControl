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

// ����·����ʾ�����ļ��б�
BOOL CRemoteFileDlg::ShowLocalFileList(CString strFilePath)
{
	list<FileInfo> lt;
	if (!GetFileInfoList(strFilePath.GetBuffer(), lt))
	{
		return FALSE;
	}

	// ����б�
	m_localFileList.DeleteAllItems();

	int i = 0;
	for (FileInfo info : lt)
	{
		m_localFileList.InsertItem(i, NULL);
		// �����ļ���
		m_localFileList.SetItemText(i, 0, info.strName.c_str());
		// �����ļ���С
		if (!info.strSize.empty())
		{
			m_localFileList.SetItemText(i, 1, info.strSize.c_str());
		}
		// ��������޸�ʱ��
		if (!info.strModifiedTime.empty())
		{
			m_localFileList.SetItemText(i, 2, info.strModifiedTime.c_str());
		}
		i++;
	}

	return TRUE;
}

// ����·����ʾԶ���ļ��б�
BOOL CRemoteFileDlg::ShowRemoteFileList(CString strFilePath)
{
	// ��������
	if (!SendInt(m_clientSocket, FILE_LIST))
	{
		return FALSE;
	}

	// �����ļ�·��
	if (!SendData(m_clientSocket, strFilePath, strFilePath.GetLength() + 1))
	{
		return FALSE;
	}

	// �����ļ��б�
	char* pBuf = NULL;
	int nLen = 0;
	if (!RecvData(m_clientSocket, pBuf, nLen) || -1 == nLen)
	{
		// ���ճ�����ߴ�·��ʧ��
		return FALSE;
	}
	unique_ptr<char> upBuf(pBuf);

	// ����б�
	m_remoteFileList.DeleteAllItems();

	// ����
	int i = 0;
	char* p = pBuf;
	while (p < pBuf + nLen)
	{
		m_remoteFileList.InsertItem(i, NULL);

		// �����ļ���
		m_remoteFileList.SetItemText(i, 0, p);
		p += strlen(p) + 1;
		// �����ļ���С
		m_remoteFileList.SetItemText(i, 1, p);
		p += strlen(p) + 1;
		// ��������޸�ʱ��
		m_remoteFileList.SetItemText(i, 2, p);
		p += strlen(p) + 1;

		i++;
	}

	return TRUE;
}

// ���ݵ�ǰ·�����ļ�������µ�·��
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

	// ����Զ���ļ�
	int nCmd = REMOTE_FILE;
	if (SOCKET_ERROR == send(m_clientSocket, (char*)&nCmd, sizeof(nCmd), 0))
	{
		PrintErrMsg("send");
		EndDialog(-1);
		return TRUE;
	}

	// ���ز˵���Դ
	m_menu.LoadMenu(IDR_MENU);

	// �����б�ؼ���ʽ
	m_localFileList.SetExtendedStyle(
		LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | m_localFileList.GetExtendedStyle());
	m_remoteFileList.SetExtendedStyle(
		LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | m_remoteFileList.GetExtendedStyle());

	// �������
	char* cols[] = { "����", "��С", "�޸�����" };
	int nCnt = sizeof(cols) / sizeof(char*);
	for (int i = 0; i < nCnt; i++)
	{
		m_localFileList.InsertColumn(i, cols[i], 0, 200);
		m_remoteFileList.InsertColumn(i, cols[i], 0, 200);
	}

	// ��ʾ�����̷�
	ShowLocalFileList("");

	// ��ʾԶ���̷�
	ShowRemoteFileList("");

	return TRUE;
}

// ˫���򿪱����ļ�
void CRemoteFileDlg::OnDblclkListLocalFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	// ��ȡ�ļ���
	CString strFileName = m_localFileList.GetItemText(pNMItemActivate->iItem, 0);

	// ��ȡ��·��
	CString strNewPath = GetFilePath(m_strCurLocalPath, strFileName);

	if (ShowLocalFileList(strNewPath))
	{
		// ����ʾ�ɹ�������µ�ǰ�ļ�·��
		m_strCurLocalPath = strNewPath;
		// ͬ����edit�ؼ�
		m_localPathEdit.SetWindowText(m_strCurLocalPath);
	}
}

// ˫����Զ���ļ�
void CRemoteFileDlg::OnDblclkListRemoteFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	// ��ȡ�ļ���
	CString strFileName = m_remoteFileList.GetItemText(pNMItemActivate->iItem, 0);

	// ��ȡ��·��
	CString strNewPath = GetFilePath(m_strCurRemotePath, strFileName);

	if (ShowRemoteFileList(strNewPath))
	{
		// ����ʾ�ɹ�������µ�ǰԶ���ļ�·��
		m_strCurRemotePath = strNewPath;
		// ͬ����edit�ؼ�
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
		AfxMessageBox("��ѡ���ļ�");
	}
	CString strFileName = m_localFileList.GetItemText(nRow, 0);

	// ����ָ��
	if (!SendInt(m_clientSocket, UPLOAD_FILE))
	{
		return;
	}
	// �����ļ���
	if (!SendData(m_clientSocket, strFileName, strFileName.GetLength() + 1))
	{
		return;
	}

	// �ļ�·��
	string strFilePath = m_strCurLocalPath + "\\" + strFileName;
	// ��ȡ�ļ�������
	SendFile(m_clientSocket, strFilePath);
}


void CRemoteFileDlg::OnDownload()
{
	int nRow = m_remoteFileList.GetSelectionMark();
	if (-1 == nRow)
	{
		AfxMessageBox("��ѡ���ļ�");
	}
	CString strFileName = m_remoteFileList.GetItemText(nRow, 0);

	// ����ָ��
	if (!SendInt(m_clientSocket, DOWNLOAD_FILE))
	{
		return;
	}
	// �����ļ���
	if (!SendData(m_clientSocket, strFileName, strFileName.GetLength() + 1))
	{
		return;
	}

	// �ļ�·��
	string strFilePath = m_strCurLocalPath + "\\" + strFileName;
	// �����ļ�������
	RecvFile(m_clientSocket, strFilePath);
}
