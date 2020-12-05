#pragma once
#include <Windows.h>
#include <string>
#include <list>
using namespace std;

#define SERVER_PORT 5566    // 服务器端口

enum CMD
{
    REMOTE_DESKTOP, // 远程桌面
    REMOTE_CMD,     // 远程命令行
    REMOTE_FILE     // 远程文件
};

enum RemoteFileCmd
{
    FILE_LIST,      // 获取文件列表
    UPLOAD_FILE,    // 上传文件
    DOWNLOAD_FILE   // 下载文件
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
    string strName;         // 文件名
    string strSize;         // 文件大小
    string strModifiedTime; // 最后修改时间

    FileInfo() : strName(""), strSize(""), strModifiedTime("") {}
};

void PrintErrMsg(const char* szPreMsg);
SOCKET ConnectTargetHost(DWORD dwTargetIp);

bool SendData(SOCKET s, const char* pData, int nLen);
bool SendInt(SOCKET s, int n);
bool RecvData(SOCKET s, char*& pBuf, int& nLen);
bool RecvInt(SOCKET s, int& n);

// 获取文件列表
bool GetFileInfoList(string strFilePath, list<FileInfo>& lt);
// 将文件列表序列化
int FileInfoListToByteAry(list<FileInfo>& lt, char* pBuf);
// 读取文件并发送
bool SendFile(SOCKET s, string strFilePath);
// 接收文件并保存
bool RecvFile(SOCKET s, string strFilePath);

// 压缩图片
bool CompressBitmap(const char* UncompressedData, char* CompressedData, SIZE_T UncompressedDataSize, SIZE_T& CompressedDataSize);
// 解压缩图片
bool DecompressBitmap(const char* CompressedData, char* UncompressedData, SIZE_T CompressedDataSize, SIZE_T& UncompressedDataSize);