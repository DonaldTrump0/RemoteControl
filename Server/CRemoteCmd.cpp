#include "CRemoteCmd.h"
#include "../Common/Common.h"

CRemoteCmd::CRemoteCmd(SOCKET socket) : m_socket(socket)
{
    int pParams[2] = { 0 };
    pParams[0] = (int)m_socket;
    pParams[1] = (int)&m_cmdPipe;

    HANDLE hThreads[2] = { 0 };
    hThreads[0] = CreateThread(0, 0, ReadPipeThreadProc, pParams, 0, 0);
    hThreads[1] = CreateThread(0, 0, WritePipeThreadProc, pParams, 0, 0);

    WaitForMultipleObjects(2, hThreads, FALSE, INFINITE);

    TerminateThread(hThreads[0], 0);
    TerminateThread(hThreads[1], 0);
    CloseHandle(hThreads[0]);
    CloseHandle(hThreads[1]);
}

CRemoteCmd::~CRemoteCmd()
{
    closesocket(m_socket);
}

// 从管道读取数据并发送
DWORD WINAPI CRemoteCmd::ReadPipeThreadProc(LPVOID lpParam)
{
    int* pParams = (int*)lpParam;
    SOCKET s = (SOCKET)pParams[0];
    CCmdPipe* pCmdPipe = (CCmdPipe*)pParams[1];

    while (true)
    {
        string str = pCmdPipe->ReadFromPipe();
        if (str.empty())
        {
            return 0;
        }

        if (SOCKET_ERROR == send(s, str.c_str(), str.length(), 0))
        {
            PrintErrMsg("send");
            return 0;
        }
    }

    return 0;
}

// 接收远程数据并写入管道
DWORD WINAPI CRemoteCmd::WritePipeThreadProc(LPVOID lpParam)
{
    int* pParams = (int*)lpParam;
    SOCKET clientSocket = (SOCKET)pParams[0];
    CCmdPipe* pCmdPipe = (CCmdPipe*)pParams[1];

    while (true)
    {
        char pBuf[4096] = { 0 };
        if (recv(clientSocket, pBuf, sizeof(pBuf) - 1, 0) <= 0)
        {
            PrintErrMsg("recv");
            return 0;
        }
        if (!pCmdPipe->WriteToPipe(pBuf))
        {
            return 0;
        }
    }

    return 0;
}
