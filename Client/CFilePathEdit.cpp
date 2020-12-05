#include "Client.h"
#include "CFilePathEdit.h"
#include "CRemoteFileDlg.h"
#include "../Common/Common.h"

IMPLEMENT_DYNAMIC(CFilePathEdit, CEdit)

CFilePathEdit::CFilePathEdit(BOOL bLocalPath)
	: m_bLocalPath(bLocalPath)
{
}

CFilePathEdit::~CFilePathEdit()
{
}

BEGIN_MESSAGE_MAP(CFilePathEdit, CEdit)
END_MESSAGE_MAP()


BOOL CFilePathEdit::PreTranslateMessage(MSG* pMsg)
{
	// 处理回车消息
	if (WM_KEYDOWN == pMsg->message && VK_RETURN == pMsg->wParam)
	{
		CRemoteFileDlg* pParent = (CRemoteFileDlg*)GetParent();

		CString strFilePath;
		GetWindowText(strFilePath);

		// 去除路径末尾的反斜杠
		int nEndIdx = strFilePath.GetLength() - 1;
		if (nEndIdx > 0 && '\\' == strFilePath[nEndIdx])
		{
			strFilePath.Delete(nEndIdx, 1);
		}

		// 显示本地文件列表
		if (m_bLocalPath)
		{
			if (pParent->ShowLocalFileList(strFilePath))
			{
				pParent->m_strCurLocalPath = strFilePath;
				return TRUE;
			}
		}
		// 显示远程文件列表
		else
		{
			if (pParent->ShowRemoteFileList(strFilePath))
			{
				pParent->m_strCurRemotePath = strFilePath;
				return TRUE;
			}
		}

		AfxMessageBox("系统找不到指定路径");
		return TRUE;
	}

	return CEdit::PreTranslateMessage(pMsg);
}
