#ifndef RESOURCE_MANAGER_HDR
#define RESOURCE_MANAGER_HDR

#include "Platform.h"
#include "Assert.h"
#include "HashMap.h"

namespace Air 
{
    struct ResourceManager;

    //Reference counting and named resources.
    struct Resource 
    {
        void addReference() { ++references; }
        void removeReference() { AIR_ASSERT(references != 0); --references; }

        uint64_t references = 0;
        const char* name = nullptr;
    };

    //TODO: Look up what a resource compiler is meant to do.
    struct ResourceCompiler 
    {
        
    };

    //Note: There is a interface class to here in the OG code but I'm not doing it.
    //struct ResourceLoader = Interface class
    //struct ResourceFilenameResolver = Interface class.

    struct ResourceManager 
    {
        void init(Allocator* alloc);
    };
}

#endif // !RESOURCE_MANAGER_HDR
