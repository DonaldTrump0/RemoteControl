#include "Common.h"
#include <stdio.h>
#include <iostream>
#include <compressapi.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Cabinet.lib")

void PrintErrMsg(const char* szPreMsg)
{
    char szErrMsg[256] = { 0 };
    char buf[256] = { 0 };
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, szErrMsg, sizeof(szErrMsg), 0);
    sprintf(buf, "%s: %s\r\n", szPreMsg, szErrMsg);
    OutputDebugString(buf);
}

SOCKET ConnectTargetHost(DWORD dwTargetIp)
{
	// ��ʼ���׽���(ʹ��TCPЭ��)
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == clientSocket)
	{
		PrintErrMsg("socket init");
		return INVALID_SOCKET;
	}

	// ���ӷ�����(��������)
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(dwTargetIp);
	if (INVALID_SOCKET == connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		PrintErrMsg("connect");
		closesocket(clientSocket);
		return INVALID_SOCKET;
	}

	return clientSocket;
}

bool SendData(SOCKET s, const char* pData, int nLen)
{
	// ���ͳ���
	if (SOCKET_ERROR == send(s, (char*)&nLen, sizeof(int), 0))
	{
		PrintErrMsg("send");
		return false;
	}

	// ��������
	if (SOCKET_ERROR == send(s, pData, nLen, 0))
	{
		PrintErrMsg("send");
		return false;
	}

	return true;
}

bool SendInt(SOCKET s, int n)
{
	if (SOCKET_ERROR == send(s, (char*)&n, sizeof(int), 0))
	{
		PrintErrMsg("send");
		return false;
	}

	return true;
}

bool RecvData(SOCKET s, char*& pBuf, int& nLen)
{
	// ���ճ���
	if (recv(s, (char*)&nLen, sizeof(int), 0) <= 0)
	{
		PrintErrMsg("recv");
		return false;
	}

	if (nLen <= 0)
	{
		return true;
	}

	pBuf = new char[nLen];

	// ��������
	int nRecvBytes = 0;
	while (nRecvBytes < nLen)
	{
		int nRet = recv(s, pBuf + nRecvBytes, nLen - nRecvBytes, 0);
		if (nRet <= 0)
		{
			PrintErrMsg("recv");
			delete pBuf;
			return false;
		}
		nRecvBytes += nRet;
	}

	return true;
}

bool RecvInt(SOCKET s, int& n)
{
	if (recv(s, (char*)&n, sizeof(int), 0) <= 0)
	{
		PrintErrMsg("recv");
		return false;
	}
	
	return true;
}

bool GetFileInfoList(string strFilePath, list<FileInfo>& lt)
{
	// ȥ���ļ�·��ĩβ�ķ�б��
	int nEndIdx = strFilePath.length() - 1;
	if (nEndIdx >= 0 && '\\' == strFilePath[nEndIdx])
	{
		strFilePath[nEndIdx] = 0;
	}

	// ��·��Ϊ�գ����ѯ�����̷�
	if ("" == strFilePath)
	{
		char pBuf[MAXBYTE] = { 0 };
		char* pDriver = pBuf;

		if (0 == GetLogicalDriveStrings(MAXBYTE, pBuf))
		{
			PrintErrMsg("GetLogicalDriveStrings");
			return false;
		}

		while (0 != *pDriver)
		{
			// ȥ����б��
			pDriver[2] = 0;

			FileInfo fileInfo;
			fileInfo.strName = pDriver;
			lt.push_back(fileInfo);

			pDriver += 4;
		}
		return true;
	}

	// ��ѯ�ļ�
	strFilePath += "\\*";
	WIN32_FIND_DATA fileData = { 0 };
	HANDLE hFind = FindFirstFile(strFilePath.c_str(), &fileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return false;
	}

	do
	{
		if (1 == strlen(fileData.cFileName) && '.' == fileData.cFileName[0])
		{
			// ȥ��"."�ļ�
			continue;
		}

		char p[64] = { 0 };
		FileInfo info;

		// �ļ���
		info.strName = fileData.cFileName;

		// �ļ���û���ļ���С����
		if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			// �ļ���С
			LARGE_INTEGER fileSize = { 0 };
			fileSize.HighPart = fileData.nFileSizeHigh;
			fileSize.LowPart = fileData.nFileSizeLow;
			sprintf(p, "%llu Bytes", fileSize.QuadPart);
			info.strSize = p;
		}

		// ����޸�ʱ��
		FILETIME localFileTime = { 0 };
		SYSTEMTIME systemTime = { 0 };
		FileTimeToLocalFileTime(&fileData.ftLastWriteTime, &localFileTime);
		FileTimeToSystemTime(&localFileTime, &systemTime);
		sprintf(p, "%d/%02d/%02d %02d:%02d",
			systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute);
		info.strModifiedTime = p;

		lt.push_back(info);
	} while (FindNextFile(hFind, &fileData));

	// �رղ�ѯ�ļ����
	FindClose(hFind);

	return true;
}

int FileInfoListToByteAry(list<FileInfo>& lt, char* pBuf)
{
	char* p = pBuf;
	for (FileInfo info : lt)
	{
		// �ļ���
		strcpy(p, info.strName.c_str());
		p += strlen(p) + 1;

		// �ļ���С
		strcpy(p, info.strSize.c_str());
		p += strlen(p) + 1;

		// ����޸�ʱ��
		strcpy(p, info.strModifiedTime.c_str());
		p += strlen(p) + 1;
	}
	return p - pBuf;
}

bool SendFile(SOCKET s, string strFilePath)
{
	// ���ļ�
	HANDLE hFile = CreateFile(strFilePath.c_str()
		, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		PrintErrMsg("CreateFile");
		MessageBox(0, "���ļ�ʧ��", 0, 0);
		return false;
	}

	// �ļ���С
	int nFileSize = GetFileSize(hFile, NULL);
	// ��ȡ�ļ�
	char* pBuf = new char[nFileSize];
	unique_ptr<char> upBuf(pBuf);
	if (!ReadFile(hFile, pBuf, nFileSize, NULL, NULL))
	{
		PrintErrMsg("ReadFile");
		MessageBox(0, "��ȡ�ļ�ʧ��", 0, 0);
		CloseHandle(hFile);
		return false;
	}

	// �����ļ�
	if (!SendData(s, pBuf, nFileSize))
	{
		CloseHandle(hFile);
		return false;
	}

	// �ر��ļ�
	CloseHandle(hFile);

	return true;
}

bool RecvFile(SOCKET s, string strFilePath)
{
	// �����ļ�
	char* pBuf = NULL;
	int nLen = 0;
	if (!RecvData(s, pBuf, nLen))
	{
		return false;
	}
	unique_ptr<char> upBuf(pBuf);

	// ���������ļ�
	HANDLE hFile = CreateFile(strFilePath.c_str()
		, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		PrintErrMsg("CreateFile");
		MessageBox(0, "�����ļ�ʧ��", 0, 0);
		return false;
	}

	// д���ļ�
	if (!WriteFile(hFile, pBuf, nLen, NULL, NULL))
	{
		PrintErrMsg("WriteFile");
		MessageBox(0, "д���ļ�ʧ��", 0, 0);
		CloseHandle(hFile);
		return false;
	}

	// �ر��ļ�
	CloseHandle(hFile);

	return true;
}

bool CompressBitmap(const char* UncompressedData, char* CompressedData, SIZE_T UncompressedDataSize, SIZE_T& CompressedDataSize)
{
	COMPRESSOR_HANDLE hCompressor = NULL;

	//  Create an XpressHuff compressor.
	BOOL bSuccess = CreateCompressor(
		COMPRESS_ALGORITHM_XPRESS_HUFF, //  Compression Algorithm
		NULL,                           //  Optional allocation routine
		&hCompressor);                  //  Handle

	if (!bSuccess)
	{
		PrintErrMsg("CreateCompressor");
		goto done;
	}

	//  Query compressed buffer size.
	bSuccess = Compress(
		hCompressor,                 //  hCompressor Handle
		UncompressedData,            //  Input buffer, Uncompressed data
		UncompressedDataSize,        //  Uncompressed data size
		NULL,                        //  Compressed Buffer
		0,                           //  Compressed Buffer size
		&CompressedDataSize);        //  Compressed Data size

	if (!bSuccess)
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			PrintErrMsg("Compress");
			goto done;
		}
	}

	//  Call Compress() again to do real compression and output the compressed data to CompressedData.
	bSuccess = Compress(
		hCompressor,          //  hCompressor Handle
		UncompressedData,     //  Input buffer, Uncompressed data
		UncompressedDataSize, //  Uncompressed data size
		CompressedData,       //  Compressed Buffer
		CompressedDataSize,   //  Compressed Buffer size
		&CompressedDataSize); //  Compressed Data size

	if (!bSuccess)
	{
		PrintErrMsg("Compress");
		goto done;
	}

	return true;

done:
	if (hCompressor != NULL)
	{
		CloseCompressor(hCompressor);
	}
	return false;
}

bool DecompressBitmap(const char* CompressedData, char* UncompressedData, SIZE_T CompressedDataSize, SIZE_T& UncompressedDataSize)
{
	DECOMPRESSOR_HANDLE hDecompressor = NULL;

	//  Create an XpressHuff decompressor.
	BOOL bSuccess = CreateDecompressor(
		COMPRESS_ALGORITHM_XPRESS_HUFF,	//  Compression Algorithm
		NULL,                           //  Optional allocation routine
		&hDecompressor);                //  Handle

	if (!bSuccess)
	{
		PrintErrMsg("CreateDecompressor");
		goto done;
	}

	//  Query decompressed buffer size.
	bSuccess = Decompress(
		hDecompressor,             //  hCompressor Handle
		CompressedData,            //  Compressed data
		CompressedDataSize,        //  Compressed data size
		NULL,                      //  Buffer set to NULL
		0,                         //  Buffer size set to 0
		&UncompressedDataSize);    //  Decompressed Data size

	if (!bSuccess)
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			PrintErrMsg("Decompress");
			goto done;
		}
	}

	//  Decompress data and write data to DecompressedBuffer.
	bSuccess = Decompress(
		hDecompressor,              //  hDecompressor handle
		CompressedData,             //  Compressed data
		CompressedDataSize,         //  Compressed data size
		UncompressedData,           //  Decompressed buffer
		UncompressedDataSize,       //  Decompressed buffer size
		&UncompressedDataSize);	    //  Decompressed data size

	if (!bSuccess)
	{
		PrintErrMsg("Decompress");
		goto done;
	}

	return true;

done:
	if (hDecompressor != NULL)
	{
		CloseCompressor(hDecompressor);
	}
	return false;
}
