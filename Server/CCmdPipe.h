#pragma once
#include <Windows.h>
#include <string>
using namespace std;

class CCmdPipe
{
public:
    CCmdPipe();
    ~CCmdPipe();
    BOOL WriteToPipe(string strInput);
    string ReadFromPipe();

private:
    void CreateChildProcess();

private:
    // �ܵ����
    HANDLE m_hPipeStdinRd = NULL;
    HANDLE m_hPipeStdinWr = NULL;
    HANDLE m_hPipeStdoutRd = NULL;
    HANDLE m_hPipeStdoutWr = NULL;
    // �ӽ��̾��
    HANDLE m_hChildProcess = NULL;
};

