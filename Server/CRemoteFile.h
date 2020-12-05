#pragma once
#include <Windows.h>
#include <string>
using namespace std;

class CRemoteFile
{
public:
    CRemoteFile(SOCKET socket);
    ~CRemoteFile();

    bool OnFileList();
    bool OnUploadFile();
    bool OnDownloadFile();

private:
    SOCKET m_socket;
    string m_strCurPath;
};