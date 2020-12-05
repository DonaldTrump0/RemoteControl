#pragma once
#include <Windows.h>
#include "CCmdPipe.h"

class CRemoteCmd
{
public:
    CRemoteCmd(SOCKET socket);
    ~CRemoteCmd();

    static DWORD WINAPI ReadPipeThreadProc(LPVOID lpParam);
    static DWORD WINAPI WritePipeThreadProc(LPVOID lpParam);

private:
    SOCKET m_socket;
    CCmdPipe m_cmdPipe;
};