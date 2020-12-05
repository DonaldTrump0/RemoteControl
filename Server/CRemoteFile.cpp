#include "CRemoteFile.h"
#include "../Common/Common.h"
#include <iostream>

CRemoteFile::CRemoteFile(SOCKET socket) 
    : m_socket(socket)
    , m_strCurPath("")
{
    while (true)
    {
        // 接收远程指令
        int nCmd = 0;
        if (!RecvInt(socket, nCmd))
        {
            return;
        }

        switch (nCmd)
        {
        case FILE_LIST:
        {
            if (!OnFileList())
            {
                return;
            }
            break;
        }
        case UPLOAD_FILE:
        {
            if (!OnUploadFile())
            {
                return;
            }
            break;
        }
        case DOWNLOAD_FILE:
        {
            if (!OnDownloadFile())
            {
                return;
            }
            break;
        }
        }
    }
}

CRemoteFile::~CRemoteFile()
{
    closesocket(m_socket);
}

bool CRemoteFile::OnFileList()
{
    // 接收文件路径
    char* strFilePath = NULL;
    int nLen = 0;
    if (!RecvData(m_socket, strFilePath, nLen))
    {
        return false;
    }
    unique_ptr<char> upFilePath(strFilePath);

    // 获取文件列表
    list<FileInfo> lt;
    if (GetFileInfoList(strFilePath, lt))
    {
        char pBuf[0x1000] = { 0 };
        int nLen = FileInfoListToByteAry(lt, pBuf);
        if (!SendData(m_socket, pBuf, nLen))
        {
            return false;
        }
        // 更新当前文件路径
        m_strCurPath = strFilePath;
    }
    else
    {
        // 获取出错，发送-1
        if (!SendInt(m_socket, -1))
        {
            return false;
        }
    }

    return true;
}

bool CRemoteFile::OnUploadFile()
{
    // 接收文件名
    char* strFileName = NULL;
    int nLen = 0;
    if (!RecvData(m_socket, strFileName, nLen))
    {
        return false;
    }
    unique_ptr<char> pFileName(strFileName);

    // 文件路径
    string strFilePath = m_strCurPath + "\\" + strFileName;

    // 接收文件并保存
    return RecvFile(m_socket, strFilePath);
}

bool CRemoteFile::OnDownloadFile()
{
    // 接收文件名
    char* strFileName = NULL;
    int nLen = 0;
    if (!RecvData(m_socket, strFileName, nLen))
    {
        return false;
    }
    unique_ptr<char> upFileName(strFileName);

    // 文件路径
    string strFilePath = m_strCurPath + "\\" + strFileName;

    // 读取文件并发送
    return SendFile(m_socket, strFilePath);
}
