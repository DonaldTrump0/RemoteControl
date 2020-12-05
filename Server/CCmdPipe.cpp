#include "CCmdPipe.h"
#include "../Common/Common.h"

CCmdPipe::CCmdPipe()
{
    // 设置管道句柄可继承
    SECURITY_ATTRIBUTES sa = { 0 };
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // 为子进程的STDOUT创建管道
    if (!CreatePipe(&m_hPipeStdoutRd, &m_hPipeStdoutWr, &sa, 0))
    {
        PrintErrMsg("StdoutRd CreatePipe");
        return;
    }

    // 为子进程的STDIN创建管道
    if (!CreatePipe(&m_hPipeStdinRd, &m_hPipeStdinWr, &sa, 0))
    {
        PrintErrMsg("Stdin CreatePipe");
        return;
    }

    // 创建子进程
    CreateChildProcess();
}

CCmdPipe::~CCmdPipe()
{
    CloseHandle(m_hPipeStdinRd);
    CloseHandle(m_hPipeStdinWr);
    CloseHandle(m_hPipeStdoutRd);
    CloseHandle(m_hPipeStdoutWr);

    // 结束子进程
    TerminateProcess(m_hChildProcess, 0);
    CloseHandle(m_hChildProcess);
}

BOOL CCmdPipe::WriteToPipe(string strInput)
{
    DWORD dwWritten;
    if (!WriteFile(m_hPipeStdinWr, strInput.c_str(), strInput.length(), &dwWritten, 0))
    {
        PrintErrMsg("WriteFile");
        return FALSE;
    }
    return TRUE;
}

string CCmdPipe::ReadFromPipe()
{
    DWORD dwRead;
    CHAR pBuf[4096] = { 0 };

    if (!ReadFile(m_hPipeStdoutRd, pBuf, sizeof(pBuf), &dwRead, 0))
    {
        PrintErrMsg("ReadFile");
        return "";
    }

    return string(pBuf);
}

void CCmdPipe::CreateChildProcess()
{
    char szCmdline[] = "cmd /Q";
    PROCESS_INFORMATION pi = { 0 };
    STARTUPINFO si = { 0 };

    si.cb = sizeof(STARTUPINFO);
    si.wShowWindow = SW_HIDE;
    si.hStdError = m_hPipeStdoutWr;
    si.hStdOutput = m_hPipeStdoutWr;
    si.hStdInput = m_hPipeStdinRd;
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

    if (!CreateProcess(0, szCmdline, 0, 0, TRUE, 0, 0, 0, &si, &pi))
    {
        PrintErrMsg("CreateProcess");
        return;
    }

    m_hChildProcess = pi.hProcess;
}
