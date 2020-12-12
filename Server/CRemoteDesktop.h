#pragma once
#include <Windows.h>

// ����ͼƬ������
#define INTERVAL_TIME 100

class CRemoteDesktop
{
public:
    CRemoteDesktop(SOCKET desktopSocket, SOCKET cmdSocket);
    ~CRemoteDesktop();

    static DWORD WINAPI DesktopThreadProc(LPVOID lpParam);
    static DWORD WINAPI CmdThreadProc(LPVOID lpParam);

private:
    SOCKET m_desktopSocket;
    SOCKET m_cmdSocket;
};