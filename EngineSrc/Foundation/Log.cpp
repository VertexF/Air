#include "Log.h"

#if defined(_MSC_VER)
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

#include <stdio.h>
#include <stdarg.h>

namespace Air
{
    LogService sLogService;
    static constexpr int32_t STRING_BUFFER_SIZE = 1024 * 1024;
    static char LOG_BUFFER[STRING_BUFFER_SIZE];

    static void outputConsole(char* logBuffer) 
    {
        printf("%s", logBuffer);
    }

#if defined(_MSC_VER)
    static void outputVisualStudio(char* logBuffer) 
    {
        OutputDebugStringA(logBuffer);
    }
#endif

    LogService* LogService::instance() 
    {
        return &sLogService;
    }

    void LogService::printFormat(const char* format ...)
    {
        va_list args;

        va_start(args, format);

    #if defined(_MSC_VER)
        vsnprintf_s(LOG_BUFFER, ArraySize(LOG_BUFFER), format, args);
    #else
        vsnprintf(LOG_BUFFER, ArraySize(LOG_BUFFER), format, args);
    #endif

        LOG_BUFFER[ArraySize(LOG_BUFFER) - 1] = '\0';
        va_end(args);

        outputConsole(LOG_BUFFER);

    #if defined(_MSC_VER)
        outputVisualStudio(LOG_BUFFER);
    #endif //_MSC_VER

        if (printCallback) 
        {
            printCallback(LOG_BUFFER);
        }
    }

    void LogService::setCallback(PrintCallback callback) 
    {
        printCallback = callback;
    }
}//Air