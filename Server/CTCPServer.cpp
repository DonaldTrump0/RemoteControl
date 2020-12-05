#include <stdio.h>
#include <iostream>
#include <string>
#include "CTCPServer.h"
#include "../Common/Common.h"
#include "CRemoteDesktop.h"
#include "CRemoteCmd.h"
#include "CRemoteFile.h"
using namespace std;

int main()
{
    CTCPServer tcpServer;
    return 0;
}

CTCPServer::CTCPServer()
{
    // ��ʼ���׽��ֿ�
    WSADATA wsaData;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        PrintErrMsg("WSAStartup");
        return;
    }

    // ��ʼ���׽���(ʹ��TCPЭ��)
    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == m_listenSocket)
    {
        PrintErrMsg("socket init");
        return;
    }

    // �󶨶˿�
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if (SOCKET_ERROR == bind(m_listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)))
    {
        PrintErrMsg("bind");
        return;
    }

    // ����
    if (SOCKET_ERROR == listen(m_listenSocket, SOMAXCONN))
    {
        PrintErrMsg("listen");
        return;
    }

    // ��ӡ����IP
    PrintHostIp();

    while (true)
    {
        // ���ܿͻ�������
        sockaddr_in clientAddr;
        int nLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(m_listenSocket, (sockaddr*)&clientAddr, &nLen);
        if (SOCKET_ERROR == clientSocket)
        {
            PrintErrMsg("accept");
            return;
        }
        printf("accept ip = %s, port = %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // ����ָ��
        int nCmd = 0;
        if (recv(clientSocket, (char*)&nCmd, sizeof(nCmd), 0) <= 0)
        {
            PrintErrMsg("recv");
            return;
        }

        switch (nCmd)
        {
        // Զ������
        case REMOTE_DESKTOP:
        {
            // �ٽ�һ������
            sockaddr_in clientAddr;
            int nLen = sizeof(sockaddr_in);
            SOCKET cmdSocket = accept(m_listenSocket, (sockaddr*)&clientAddr, &nLen);
            if (SOCKET_ERROR == cmdSocket)
            {
                PrintErrMsg("accept");
                return;
            }
            int* pParams = new int[2];
            pParams[0] = clientSocket;
            pParams[1] = cmdSocket;

            CreateThread(0, 0, RemoteDesktopThreadProc, (LPVOID)pParams, 0, 0);
            break;
        }
        // Զ��������
        case REMOTE_CMD:
        {
            CreateThread(0, 0, RemoteCmdThreadProc, (LPVOID)clientSocket, 0, 0);
            break;
        }
        // Զ���ļ�
        case REMOTE_FILE:
            CreateThread(0, 0, RemoteFileThreadProc, (LPVOID)clientSocket, 0, 0);
            break;
        }
    }
}

CTCPServer::~CTCPServer()
{
    closesocket(m_listenSocket);

    // ����ʼ����
    WSACleanup();
}

BOOL CTCPServer::PrintHostIp()
{
    // ��ȡ������  
    char hostname[256];
    int ret = gethostname(hostname, sizeof(hostname));
    if (ret == SOCKET_ERROR)
    {
        return false;
    }
    // ��ȡ����ip  
    HOSTENT* host = gethostbyname(hostname);
    if (host == NULL)
    {
        return false;
    }
    // ת��Ϊchar*
    char ip[256];
    strcpy(ip, inet_ntoa(*(in_addr*)*host->h_addr_list));

    in_addr* pAddr = (in_addr*)*host->h_addr_list;

    for (int i = 0; i < (strlen((char*)*host->h_addr_list) - strlen(host->h_name)) / 4 && pAddr; i++)
    {
        printf("%s\r\n", inet_ntoa(pAddr[i]));
    }
}

DWORD WINAPI CTCPServer::RemoteDesktopThreadProc(LPVOID lpParam)
{
    int* p = (int*)lpParam;
    CRemoteDesktop remoteDesktop(p[0], p[1]);
    delete[] p;
    return 0;
}

// Զ��������
DWORD WINAPI CTCPServer::RemoteCmdThreadProc(LPVOID lpParam)
{
    CRemoteCmd remoteCmd((SOCKET)lpParam);
    return 0;
}

// Զ���ļ�
DWORD WINAPI CTCPServer::RemoteFileThreadProc(LPVOID lpParam)
{
    CRemoteFile remoteFile((SOCKET)lpParam);
    return 0;
}