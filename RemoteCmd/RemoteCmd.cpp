#include <stdio.h>
#include <Windows.h>
#include "../Common/Common.h"
#pragma comment(lib, "Ws2_32.lib")

void PrintErrMsg(const char* szPreMsg)
{
	char* pMsgBuf = NULL;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&pMsgBuf, 0, NULL);
	printf("%s: %s\r\n", szPreMsg, pMsgBuf);
	LocalFree(pMsgBuf);
}

void ErrorExit(const char* szPreMsg)
{
	PrintErrMsg(szPreMsg);
	system("pause");
	ExitProcess(1);
}

// ����Զ�������߳�
DWORD WINAPI RecvRemoteCmdThreadProc(LPVOID lpParam)
{
	SOCKET clientSocket = (SOCKET)lpParam;

	while (true)
	{
		// ��������
		char pBuf[4096] = { 0 };
		if (recv(clientSocket, pBuf, sizeof(pBuf) - 1, 0) <= 0)
		{
			ErrorExit("recv");
		}

		// ��ӡ������̨
		printf("%s", pBuf);
	}

	return 0;
}

int main(int argc, char* argv[])
{
	printf("Waiting for connect...\n");

	// Ŀ������IP
	DWORD dwTargetIp = 0;
	sscanf(argv[1], "%u", &dwTargetIp);

    // ��ʼ���׽��ֿ�
    WSADATA wsaData;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
		ErrorExit("WSAStartup");
    }

	// ��ʼ���׽���(ʹ��TCPЭ��)
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == clientSocket)
	{
		ErrorExit("socket init");
	}

	// ���ӷ�����(��������)
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5566);
	serverAddr.sin_addr.S_un.S_addr = htonl(dwTargetIp);
	if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == INVALID_SOCKET)
	{
		ErrorExit("connect");
	}

	// ��Զ����������REMOTE_CMD
	int nCmd = REMOTE_CMD;
	if (SOCKET_ERROR == send(clientSocket, (char*)&nCmd, sizeof(nCmd), 0))
	{
		ErrorExit("send");
	}

	// �������������߳�
	CreateThread(0, 0, RecvRemoteCmdThreadProc, (LPVOID)clientSocket, 0, 0);

	while (true)
	{
		// �ӿ���̨������
		char pBuf[4096] = { 0 };
		gets_s(pBuf);
		strcat(pBuf, "\r\n");

		// ��������
		if (SOCKET_ERROR == send(clientSocket, pBuf, strlen(pBuf), 0))
		{
			ErrorExit("send");
		}
	}

    return 0;
}