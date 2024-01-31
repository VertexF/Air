#include "Process.h"
#include "Assert.h"
#include "Log.h"

#include "Memory.h"
#include "String.h"

#include <stdio.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace Air 
{
    //Static buffer to log the error coming from windows.
    static const uint32_t PROCESS_LOG_BUFFER_SIZE = 256;
    char PROCESS_LOG_BUFFER[PROCESS_LOG_BUFFER_SIZE];
    static char PROCESS_OUTPUT_BUFFER;

#if defined(_WIN32)
    void win32GetError(char* buffer, uint32_t size) 
    {
        DWORD errorCode = GetLastError();

        char* errorString = nullptr;
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, errorCode,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorString, 0, nullptr) == false) 
        {
            return;
        }

        sprintf_s(buffer, size, "%s", errorString);

        LocalFree(errorString);
    }

    bool processExcute(const char* workingDirectory, const char* processFullPath, const char* arguments, const char* searchError)
    {
        //This has been 'inspired' by a stackover flow answer about how to read stuff from a cmd exe process.
        HANDLE handleStdinPipeRead = nullptr;
        HANDLE handleStdinPipeWrite = nullptr;
        HANDLE handleStdoutPipeRead = nullptr;
        HANDLE handleStdoutPipeWrite = nullptr;

        SECURITY_ATTRIBUTES securityAttributes = { sizeof(SECURITY_ATTRIBUTES), nullptr, true };

        bool ok = CreatePipe(&handleStdinPipeRead, &handleStdinPipeWrite, &securityAttributes, 0);
        if (ok == false) 
        {
            return false;
        }

        ok = CreatePipe(&handleStdoutPipeRead, &handleStdoutPipeWrite, &securityAttributes, 0);
        if (ok == false) 
        {
            return false;
        }

        //Create startup information with std redirection.
        STARTUPINFOA startupInfo = {};
        startupInfo.cb = sizeof(startupInfo);
        startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        startupInfo.hStdInput = handleStdinPipeRead;
        startupInfo.hStdError = handleStdinPipeWrite;
        startupInfo.hStdOutput = handleStdinPipeWrite;
        startupInfo.wShowWindow = SW_SHOW;

        bool executeSuccess = false;
        PROCESS_INFORMATION processInfo = {};
        bool inheritHandles = true;
        if(CreateProcessA(processFullPath, (char*)arguments, 0, 0, inheritHandles, 0, 0, workingDirectory, &startupInfo, &processInfo))
        {
            CloseHandle(processInfo.hThread);
            CloseHandle(processInfo.hProcess);

            executeSuccess = true;
        }
        else 
        {
            win32GetError(&PROCESS_LOG_BUFFER[0], PROCESS_LOG_BUFFER_SIZE);

            aprint("Execute process error.\n Exe \"%s\" - Args: \"%s\" - Work Dir: \"%s\"\n", processFullPath, arguments, workingDirectory);
            aprint("Message: %s\n", PROCESS_LOG_BUFFER);
        }
        CloseHandle(handleStdinPipeRead);
        CloseHandle(handleStdinPipeWrite);

        //Output
        DWORD bytesRead;
        ok = ReadFile(handleStdoutPipeRead, PROCESS_LOG_BUFFER, 1024, &bytesRead, nullptr);

        //Consume all output
        //Terinate current read initalise the next.
        while (ok == true) 
        {
            PROCESS_LOG_BUFFER[bytesRead] = 0;
            aprint("%s", PROCESS_LOG_BUFFER);

            ok = ReadFile(handleStdoutPipeRead, PROCESS_LOG_BUFFER, 1024, &bytesRead, nullptr);
        }

        if (strlen(searchError) > 0 && strstr(PROCESS_LOG_BUFFER, searchError))
        {
            executeSuccess = false;
        }

        aprint("\n");

        CloseHandle(handleStdoutPipeRead);
        CloseHandle(handleStdoutPipeWrite);

        DWORD processExitCode = 0;
        GetExitCodeProcess(processInfo.hProcess, &processExitCode);

        return executeSuccess;
    }

    const char* processGetOutput() 
    {
        return PROCESS_LOG_BUFFER;
    }

#else
#endif

    bool processExcute(const char* workingDirectory, const char* processFullPath, const char* arguments, const char* searchError) 
    {
        return false;
    }

    const char* processGetOutput() 
    {
        return nullptr;
    }
}