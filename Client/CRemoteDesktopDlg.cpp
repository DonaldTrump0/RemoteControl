#include "framework.h"
#include "Client.h"
#include "CRemoteDesktopDlg.h"
#include "afxdialogex.h"
#include "ClientDlg.h"
#include "../Common/Common.h"
#include <iostream>
using namespace std;

IMPLEMENT_DYNAMIC(CRemoteDesktopDlg, CDialogEx)

CRemoteDesktopDlg::CRemoteDesktopDlg(DWORD dwTargetIp, CWnd* pParent /*=nullptr*/)
	: m_dwTargetIp(dwTargetIp)
	, CDialogEx(IDD_DIALOG_REMOTE_DESKTOP, pParent)
{
}

CRemoteDesktopDlg::~CRemoteDesktopDlg()
{
	closesocket(m_desktopSocket);
	closesocket(m_cmdSocket);
}

void CRemoteDesktopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRemoteDesktopDlg, CDialogEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
END_MESSAGE_MAP()

bool CRemoteDesktopDlg::Init()
{
	// ����
	m_desktopSocket = ConnectTargetHost(m_dwTargetIp);
	if (INVALID_SOCKET == m_desktopSocket)
	{
		return false;
	}

	// ����ָ��
	if (!SendInt(m_desktopSocket, REMOTE_DESKTOP))
	{
		return false;
	}

	// �ٽ���һ�����Ӵ�������������
	m_cmdSocket = ConnectTargetHost(m_dwTargetIp);
	if (INVALID_SOCKET == m_cmdSocket)
	{
		return false;
	}

	return true;
}

// ����Զ������
DWORD WINAPI CRemoteDesktopDlg::DesktopThreadProc(LPVOID lpThreadParameter)
{
	CRemoteDesktopDlg* pRemoteDesktopDlg = (CRemoteDesktopDlg*)lpThreadParameter;
	SOCKET s = pRemoteDesktopDlg->m_desktopSocket;

	// ����Զ������߶ȺͿ��
	int nWidth = 0;
	if (!RecvInt(s, nWidth))
	{
		return 0;
	}
	int nHeight = 0;
	if (!RecvInt(s, nHeight))
	{
		return 0;
	}

	int nNumberOfBytes = nWidth * nHeight * sizeof(COLORREF);
	char* pBytes = new char[nNumberOfBytes];
	char* pCompressedData = new char[nNumberOfBytes / 10];
	unique_ptr<char> upBytes(pBytes);
	unique_ptr<char> upCompressedData(pCompressedData);

	while (true)
	{
		// ��������ͼƬ
		int nCompressedDataSize = 0;
		if (!RecvInt(s, nCompressedDataSize))
		{
			return 0;
		}

		int nRecvBytes = 0;
		while (nRecvBytes < nCompressedDataSize)
		{
			int nRet = recv(s, pCompressedData + nRecvBytes, nCompressedDataSize - nRecvBytes, 0);
			if (nRet <= 0)
			{
				return 0;
			}
			nRecvBytes += nRet;
		}

		// ��ѹ��
		SIZE_T UncompressedDataSize = 0;
		if (!DecompressBitmap(pCompressedData, pBytes, nCompressedDataSize, UncompressedDataSize))
		{
			return 0;
		}

		// ��ʾ
		pRemoteDesktopDlg->ShowScreen(pBytes, nWidth, nHeight);
	}

	return 0;
}

// ��ʾ����
void CRemoteDesktopDlg::ShowScreen(char* pBits, int nWidth, int nHeight)
{
	CDC* pDC = GetDC();

	CDC MemDC;
	MemDC.CreateCompatibleDC(pDC);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, nWidth, nHeight);
	bitmap.SetBitmapBits(nWidth * nHeight * sizeof(COLORREF), pBits);
	MemDC.SelectObject(bitmap);

	CRect rect;
	GetClientRect(&rect);
	pDC->StretchBlt(0, 0, rect.Width(), rect.Height(), &MemDC, 0, 0, nWidth, nHeight, SRCCOPY);
}

// ���������Ϣ
void CRemoteDesktopDlg::SendMouseCmd(int nCmd, CPoint point)
{
	// ��ȡ���ڴ�С
	CRect rect;
	GetClientRect(&rect);

	struct
	{
		WORD x;
		WORD y;
	} pt;
	pt.x = (double)point.x / rect.Width() * 65536;
	pt.y = (double)point.y / rect.Height() * 65536;
	
	if (!SendInt(m_cmdSocket, nCmd))
	{
		return;
	}
	if (!SendInt(m_cmdSocket, *(int*)&pt))
	{
		return;
	}
}

// ���ͼ�����Ϣ
void CRemoteDesktopDlg::SendKeyboardCmd(int nCmd, int nKey)
{
	if (!SendInt(m_cmdSocket, nCmd))
	{
		return;
	}
	if (!SendInt(m_cmdSocket, nKey))
	{
		return;
	}
}

BOOL CRemoteDesktopDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �����ʾ����
	ShowWindow(SW_MAXIMIZE);

	// ���洫���߳�
	CreateThread(0, 0, DesktopThreadProc, this, 0, 0);

	return TRUE;
}

// point������ڴ������Ͻǵ�������ص�
void CRemoteDesktopDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	SendMouseCmd(L_BUTTON_DOWN, point);
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CRemoteDesktopDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	SendMouseCmd(L_BUTTON_UP, point);
	CDialogEx::OnLButtonUp(nFlags, point);
}


void CRemoteDesktopDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	SendMouseCmd(L_BUTTON_DBL_CLK, point);
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CRemoteDesktopDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((nFlags & MK_LBUTTON) || (nFlags & MK_RBUTTON))
	{
		SendMouseCmd(MOUSE_MOVE, point);
	}
	CDialogEx::OnMouseMove(nFlags, point);
}


void CRemoteDesktopDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	SendMouseCmd(R_BUTTON_DOWN, point);
	CDialogEx::OnRButtonDown(nFlags, point);
}


void CRemoteDesktopDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	SendMouseCmd(R_BUTTON_UP, point);
	CDialogEx::OnRButtonUp(nFlags, point);
}


void CRemoteDesktopDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SendKeyboardCmd(KEY_DOWN, nChar);
	CDialogEx::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CRemoteDesktopDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SendKeyboardCmd(KEY_UP, nChar);
	CDialogEx::OnKeyUp(nChar, nRepCnt, nFlags);
}


BOOL CRemoteDesktopDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	SendMouseCmd(MOUSE_WHEEL, pt);
	SendInt(m_cmdSocket, zDelta);
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}


void CRemoteDesktopDlg::PostNcDestroy()
{
	CDialogEx::PostNcDestroy();
	delete this;
}


void CRemoteDesktopDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	SendMouseCmd(MID_BUTTON_DOWN, point);
	CDialogEx::OnMButtonDown(nFlags, point);
}


void CRemoteDesktopDlg::OnMButtonUp(UINT nFlags, CPoint point)
{
	SendMouseCmd(MID_BUTTON_UP, point);
	CDialogEx::OnMButtonUp(nFlags, point);
}
