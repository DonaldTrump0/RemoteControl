#include "CRemoteFile.h"
#include "../Common/Common.h"
#include <iostream>

CRemoteFile::CRemoteFile(SOCKET socket) 
    : m_socket(socket)
    , m_strCurPath("")
{
    while (true)
    {
        // ����Զ��ָ��
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
    // �����ļ�·��
    char* strFilePath = NULL;
    int nLen = 0;
    if (!RecvData(m_socket, strFilePath, nLen))
    {
        return false;
    }
    unique_ptr<char> upFilePath(strFilePath);

    // ��ȡ�ļ��б�
    list<FileInfo> lt;
    if (GetFileInfoList(strFilePath, lt))
    {
        char pBuf[0x1000] = { 0 };
        int nLen = FileInfoListToByteAry(lt, pBuf);
        if (!SendData(m_socket, pBuf, nLen))
        {
            return false;
        }
        // ���µ�ǰ�ļ�·��
        m_strCurPath = strFilePath;
    }
    else
    {
        // ��ȡ��������-1
        if (!SendInt(m_socket, -1))
        {
            return false;
        }
    }

    return true;
}

bool CRemoteFile::OnUploadFile()
{
    // �����ļ���
    char* strFileName = NULL;
    int nLen = 0;
    if (!RecvData(m_socket, strFileName, nLen))
    {
        return false;
    }
    unique_ptr<char> pFileName(strFileName);

    // �ļ�·��
    string strFilePath = m_strCurPath + "\\" + strFileName;

    // �����ļ�������
    return RecvFile(m_socket, strFilePath);
}

bool CRemoteFile::OnDownloadFile()
{
    // �����ļ���
    char* strFileName = NULL;
    int nLen = 0;
    if (!RecvData(m_socket, strFileName, nLen))
    {
        return false;
    }
    unique_ptr<char> upFileName(strFileName);

    // �ļ�·��
    string strFilePath = m_strCurPath + "\\" + strFileName;

    // ��ȡ�ļ�������
    return SendFile(m_socket, strFilePath);
}
