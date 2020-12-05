#pragma once
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")

class CTCPServer
{
public:
    CTCPServer();
    ~CTCPServer();
    BOOL PrintHostIp();

    // Զ������
    static DWORD WINAPI RemoteDesktopThreadProc(LPVOID lpParam);
    // Զ��������
    static DWORD WINAPI RemoteCmdThreadProc(LPVOID lpParam);
    // Զ���ļ�
    static DWORD WINAPI RemoteFileThreadProc(LPVOID lpParam);

public:
    SOCKET m_listenSocket;
};
