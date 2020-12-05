#pragma once
#include <Windows.h>

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