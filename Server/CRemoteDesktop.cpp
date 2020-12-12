#include "CRemoteDesktop.h"
#include "../Common/Common.h"
#include <iostream>
using namespace std;

CRemoteDesktop::CRemoteDesktop(SOCKET desktopSocket, SOCKET cmdSocket)
    : m_desktopSocket(desktopSocket)
    , m_cmdSocket(cmdSocket)
{
    HANDLE hThreads[2] = { 0 };
    hThreads[0] = CreateThread(0, 0, DesktopThreadProc, (LPVOID)m_desktopSocket, 0, 0);
    hThreads[1] = CreateThread(0, 0, CmdThreadProc, (LPVOID)m_cmdSocket, 0, 0);

    WaitForMultipleObjects(2, hThreads, FALSE, INFINITE);

    TerminateThread(hThreads[0], 0);
    TerminateThread(hThreads[1], 0);
    CloseHandle(hThreads[0]);
    CloseHandle(hThreads[1]);
}

CRemoteDesktop::~CRemoteDesktop()
{
    closesocket(m_desktopSocket);
    closesocket(m_cmdSocket);
}

// 定时向客户端发送桌面截图
DWORD WINAPI CRemoteDesktop::DesktopThreadProc(LPVOID lpParam)
{
    SOCKET s = (SOCKET)lpParam;

    HWND hWnd = GetDesktopWindow();
    HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    // 获取监视器逻辑宽度与高度
    MONITORINFOEX miex;
    miex.cbSize = sizeof(miex);
    GetMonitorInfo(hMonitor, &miex);
    // 获取监视器物理宽度与高度
    DEVMODE dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;
    EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm);
    int nWidth = dm.dmPelsWidth;     // 1920
    int nHeight = dm.dmPelsHeight;   // 1080

    // 发送屏幕分辨率
    if (!SendInt(s, nWidth))
    {
        return 0;
    }
    if (!SendInt(s, nHeight))
    {
        return 0;
    }

    // 创建兼容DC
    HDC hdcDesktop = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcDesktop);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcDesktop, nWidth, nHeight);
    SelectObject(hdcMemDC, hBitmap);

    int nNumberOfBytes = nWidth * nHeight * sizeof(COLORREF);
    char* pBytes = new char[nNumberOfBytes];
    char* pCompressedData = new char[nNumberOfBytes];
    unique_ptr<char> upBytes(pBytes);
    unique_ptr<char> upCompressedData(pCompressedData);

    while (true)
    {
        //截图
        BitBlt(hdcMemDC, 0, 0, nWidth, nHeight, hdcDesktop, 0, 0, SRCCOPY);
        GetBitmapBits(hBitmap, nNumberOfBytes, pBytes);

        // 压缩
        SIZE_T nCompressedDataSize = 0;
        if (!CompressBitmap(pBytes, pCompressedData, nNumberOfBytes, nCompressedDataSize))
        {
            return 0;
        }

        // 发送
        if (!SendData(s, pCompressedData, nCompressedDataSize))
        {
            return 0;
        }

        Sleep(INTERVAL_TIME);
    }

    return 0;
}

// 处理键盘鼠标命令
DWORD WINAPI CRemoteDesktop::CmdThreadProc(LPVOID lpParam)
{
    SOCKET s = (SOCKET)lpParam;

    while (true)
    {
        // 接收键盘鼠标指令
        int nCmd = 0;
        if (!RecvInt(s, nCmd))
        {
            return 0;
        }

        // 接收键值或者鼠标坐标
        int n = 0;
        if (!RecvInt(s, n))
        {
            return 0;
        }
        POINT pt = { LOWORD(n), HIWORD(n) };

        switch (nCmd)
        {
        case L_BUTTON_DOWN:
        {
            mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            break;
        }
        case L_BUTTON_UP:
        {
            mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            break;
        }
        case L_BUTTON_DBL_CLK:
        {
            mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            break;
        }
        case R_BUTTON_DOWN:
        {
            mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
            break;
        }
        case R_BUTTON_UP:
        {
            mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
            break;
        }
        case MOUSE_MOVE:
        {
            mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
            break;
        }
        case MID_BUTTON_DOWN:
        {
            mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
            break;
        }
        case MID_BUTTON_UP:
        {
            mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
            break;
        }
        case KEY_DOWN:
        {
            keybd_event(n, 0, 0, 0);
            break;
        }
        case KEY_UP:
        {
            keybd_event(n, 0, KEYEVENTF_KEYUP, 0);
            break;
        }
        case MOUSE_WHEEL:
        {
            // 滚动大小
            int nDelta = 0;
            if (!RecvInt(s, nDelta))
            {
                return 0;
            }
            mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, pt.x, pt.y, 0, 0);
            mouse_event(MOUSEEVENTF_WHEEL, 0, 0, nDelta, 0);
            break;
        }
        }
    }

    return 0;
}
