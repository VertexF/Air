#ifndef LOG_HDR
#define LOG_HDR

#include "Platform.h"
#include "Service.h"

namespace Air 
{
    typedef void (*PrintCallback)(const char*);

    struct LogService : public Service
    {
        AIR_DECLARE_SERVICE(LogService);

        void printFormat(const char* format ...);
        void setCallback(PrintCallback callback);

        PrintCallback printCallback = nullptr;
        static constexpr const char* name = "Air Log Service";
    };

#if defined(_MSC_VER)
    #define aprint(format, ...)    Air::LogService::instance()->printFormat(format, __VA_ARGS__);
    #define aprintret(format, ...) Air::LogService::instance()->printFormat(format, __VA_ARGS__); Air::LogService::instance()->printFormat("\n");
#else
    #define aprint(format, ...)    Air::LogService::instance()->printFormat(format, ## __VA_ARGS__);
    #define aprintret(format, ...) Air::LogService::instance()->printFormat(format, ## __VA_ARGS__); Air::LogService::instance()->printFormat("\n");
#endif
}//Air

#endif // !LOG_HDR
