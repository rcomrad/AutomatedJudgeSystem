﻿//--------------------------------------------------------------------------------
#include "pipe_windows_process.hpp"

proc::PipeWindowsProcess::~PipeWindowsProcess() 
{
    closeHandles();
}

#define BUFFER_SIZE 65336 * 10

void
proc::PipeWindowsProcess::create() noexcept
{
    IORedirection();
    this->WindowsProcess::create();
}

void
proc::PipeWindowsProcess::IORedirection() noexcept
{
    WRITE_LOG("Rederecting_input_and_output_to_pipe");

    mIOSet = true;

    SECURITY_ATTRIBUTES securatyAttributes;
    securatyAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    securatyAttributes.lpSecurityDescriptor = NULL;
    securatyAttributes.bInheritHandle = true;

    if (!CreatePipe(&mChildSTDIN, &mThisSTDOUT, &securatyAttributes, 0))
    {
        WRITE_ERROR("PipeProcess", "IORedirection", 20, "Can't_create_pipe", "Windows");
    }

    if (!CreatePipe(&mThisSTDIN, &mChildSTDOUT, &securatyAttributes, 0))
    {
        WRITE_ERROR("PipeProcess", "IORedirection", 21, "Can't_create_pipe", "Windows");
    }
    //else if (aType == Process::IOType::MIXED) {}

    //GetStartupInfo(&mStartupInfo);
    ZeroMemory(&mStartupInfo, sizeof(STARTUPINFO));
    mStartupInfo.cb = sizeof(STARTUPINFO);
    mStartupInfo.dwFlags |= STARTF_USESTDHANDLES;

    mStartupInfo.hStdInput = mChildSTDIN;
    mStartupInfo.hStdError = mChildSTDOUT;
    mStartupInfo.hStdOutput = mChildSTDOUT;
}


void
proc::PipeWindowsProcess::read(std::string& result) noexcept
{
#ifdef PIPE_LOGS
    WRITE_LOG("Reading_from_pipe");
#endif

    const size_t bufSize = 1024;
    char buf[bufSize];
    memset(buf, 0, bufSize - 1);
    unsigned long bread = 0;
    unsigned long avail = 0;

    while (bread == 0 && avail == 0)
    {
        PeekNamedPipe(mThisSTDIN, buf, bufSize - 1, &bread, &avail, NULL);
    }

    memset(buf, 0, sizeof(buf));
    bread = bufSize;
    result.clear();
    while (bread >= bufSize - 1)
    {
        memset(buf, 0, sizeof(buf));
        ReadFile(mThisSTDIN, buf, bufSize - 1, &bread, NULL);
        result += std::string(buf);
    }

#ifdef PIPE_LOGS
    WD_END_LOG;
#endif
}

void
proc::PipeWindowsProcess::write(const std::string& aMessage) noexcept
{
#ifdef PIPE_LOGS
    WRITE_LOG("Writing_from_pipe");
#endif

    unsigned long bread;
    WriteFile(mThisSTDOUT, aMessage.c_str(), aMessage.size(), &bread, NULL);

#ifdef PIPE_LOGS
    WD_LOG("write " + std::to_string(bread) + " bites\n");
    WD_END_LOG;
#endif
}


void
proc::PipeWindowsProcess::closeHandles() noexcept
{
    if (mIOSet)
    {
        CloseHandle(mChildSTDIN);
        CloseHandle(mChildSTDOUT);
    }
}

//--------------------------------------------------------------------------------