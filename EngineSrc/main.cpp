#include "Foundation/Memory.h"
#include "Foundation/Array.h"
#include "Foundation/Platform.h"
#include "Foundation/Service.h"
#include "Foundation/ServiceManager.h"

#include <iostream>

namespace Air
{
    struct NewService : public Air::Service
    {
        static NewService* instance() 
        {
            static NewService s_input_service;
            return &s_input_service;
        }

        static constexpr const char* NAME = "I am a new Service";
    };
}
int main()
{
    Air::HeapAllocator heap;
    heap.init(32 * 1024);

    Air::ServiceManager* serviceManager = new Air::ServiceManager;
    serviceManager->init(&heap);
    auto input = serviceManager->get<Air::NewService>();

    return 0;
}