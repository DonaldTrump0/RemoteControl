#pragma once
#include <Windows.h>
#include <string>
#include <list>
using namespace std;

#define SERVER_PORT 5566    // �������˿�

enum CMD
{
    REMOTE_DESKTOP, // Զ������
    REMOTE_CMD,     // Զ��������
    REMOTE_FILE     // Զ���ļ�
};

enum RemoteFileCmd
{
    FILE_LIST,      // ��ȡ�ļ��б�
    UPLOAD_FILE,    // �ϴ��ļ�
    DOWNLOAD_FILE   // �����ļ�
};

enum MouseAndKeyboardMsg
{
    L_BUTTON_DOWN,
    L_BUTTON_UP,
    L_BUTTON_DBL_CLK,
    R_BUTTON_DOWN,
    R_BUTTON_UP,
    //R_BUTTON_DBL_CLK,
    MOUSE_MOVE,
    MID_BUTTON_DOWN,
    MID_BUTTON_UP,
    KEY_DOWN,
    KEY_UP,
    MOUSE_WHEEL
};

struct FileInfo
{
    string strName;         // �ļ���
    string strSize;         // �ļ���С
    string strModifiedTime; // ����޸�ʱ��

    FileInfo() : strName(""), strSize(""), strModifiedTime("") {}
};

void PrintErrMsg(const char* szPreMsg);
SOCKET ConnectTargetHost(DWORD dwTargetIp);

bool SendData(SOCKET s, const char* pData, int nLen);
bool SendInt(SOCKET s, int n);
bool RecvData(SOCKET s, char*& pBuf, int& nLen);
bool RecvInt(SOCKET s, int& n);

// ��ȡ�ļ��б�
bool GetFileInfoList(string strFilePath, list<FileInfo>& lt);
// ���ļ��б����л�
int FileInfoListToByteAry(list<FileInfo>& lt, char* pBuf);
// ��ȡ�ļ�������
bool SendFile(SOCKET s, string strFilePath);
// �����ļ�������
bool RecvFile(SOCKET s, string strFilePath);

// ѹ��ͼƬ
bool CompressBitmap(const char* UncompressedData, char* CompressedData, SIZE_T UncompressedDataSize, SIZE_T& CompressedDataSize);
// ��ѹ��ͼƬ
bool DecompressBitmap(const char* CompressedData, char* UncompressedData, SIZE_T CompressedDataSize, SIZE_T& UncompressedDataSize);