#pragma once
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")

class CTCPServer
{
public:
    CTCPServer();
    ~CTCPServer();
    BOOL PrintHostIp();

    // 远程桌面
    static DWORD WINAPI RemoteDesktopThreadProc(LPVOID lpParam);
    // 远程命令行
    static DWORD WINAPI RemoteCmdThreadProc(LPVOID lpParam);
    // 远程文件
    static DWORD WINAPI RemoteFileThreadProc(LPVOID lpParam);

public:
    SOCKET m_listenSocket;
};
