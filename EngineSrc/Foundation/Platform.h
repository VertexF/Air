#ifndef PLATFORM_HDR
#define PLATFORM_HDR

#include <stdint.h>

#if !defined(_MSC_VER)
    #include <signal.h>
#endif

#define ArraySize(arr) (sizeof(arr) / sizeof((arr)[0]))

#if defined(_MSC_VER)
    #define AIR_DEBUG_BREAK                    __debugbreak();
    #define AIR_DISABLE_WARNING(warningNumber) __pragma(warning(disable : warningNumber))
    #define AIR_CONCAT_OPERATOR(x, y)          x##y
#else
    #define AIR_DEBUG_BREAK                    raise(SIGTRAP);
    #define AIR_CONCAT_OPERATOR(x, y)          x y
#endif //_MSC_VER

#define AIR_STRINGISE(L)         #L
#define AIR_MARKSTRING(L)        AIR_STRINGISE(L)
#define AIR_CONCAT(x, y)         AIR_CONCAT_OPERATOR(x, y)
#define AIR_LINE_STRING          AIR_MARKSTRING(__LINE__)
#define AIR_FILELINE(MESSAGE)    __FILE__ "(" AIR_LINE_STRING ") : " MESSAGE

#define AIR_UNIQUE_SUFFIX(PARAM) AIR_CONCAT(PARAM, __LINE__)

#endif // !PLATFORM_HDR
