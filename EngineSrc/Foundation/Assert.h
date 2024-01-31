#ifndef ASSERT_HDR
#define ASSERT_HDR

#include "Log.h"

namespace Air 
{
#define AIR_ASSERT(condition) if((condition) == false) { aprint(AIR_FILELINE("FALSE\n")); AIR_DEBUG_BREAK }
#if defined(_MSC_VER)
#define AIR_ASSERTM(condition, message, ...) if((condition) == false) { aprint(AIR_FILELINE(AIR_CONCAT(message, "\n")), __VA_ARGS__); AIR_DEBUG_BREAK; }
#else
#define AIR_ASSERTM(condition, message, ...) if((condition) == false) { aprint(AIR_FILELINE(AIR_CONCAT(message, "\n")), ## __VA_ARGS__); AIR_DEBUG_BREAK; }
#endif

}//Air

#endif // !ASSERT_HDR
