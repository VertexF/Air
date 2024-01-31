#ifndef PROCESS_HDR
#define PROCESS_HDR

#include "Platform.h"

namespace Air 
{
    bool processExcute(const char* workingDirectory, const char* processFullPath, const char* arguments, const char* searchError = "");
    const char* processGetOutput();
}

#endif // !PROCESS_HDR
