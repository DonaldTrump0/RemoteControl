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
	// ����س���Ϣ
	if (WM_KEYDOWN == pMsg->message && VK_RETURN == pMsg->wParam)
	{
		CRemoteFileDlg* pParent = (CRemoteFileDlg*)GetParent();

		CString strFilePath;
		GetWindowText(strFilePath);

		// ȥ��·��ĩβ�ķ�б��
		int nEndIdx = strFilePath.GetLength() - 1;
		if (nEndIdx > 0 && '\\' == strFilePath[nEndIdx])
		{
			strFilePath.Delete(nEndIdx, 1);
		}

		// ��ʾ�����ļ��б�
		if (m_bLocalPath)
		{
			if (pParent->ShowLocalFileList(strFilePath))
			{
				pParent->m_strCurLocalPath = strFilePath;
				return TRUE;
			}
		}
		// ��ʾԶ���ļ��б�
		else
		{
			if (pParent->ShowRemoteFileList(strFilePath))
			{
				pParent->m_strCurRemotePath = strFilePath;
				return TRUE;
			}
		}

		AfxMessageBox("ϵͳ�Ҳ���ָ��·��");
		return TRUE;
	}

	return CEdit::PreTranslateMessage(pMsg);
}
