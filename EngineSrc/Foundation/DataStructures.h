#ifndef DATA_STRUCTURE_HDR
#define DATA_STRUCTURE_HDR

#include "Memory.h"
#include "Assert.h"

namespace Air 
{
    struct ResourcePool 
    {
        void init(Allocator* alloc, uint32_t poolSize, uint32_t resourceSize);
        void shutdown();

        uint32_t obtainResource();
        void releaseResource(uint32_t handle);
        void freeAllResources();

        void* accessResource(uint32_t index);
        const void* accessResource(uint32_t index) const;

        uint8_t* memory = nullptr;
        uint32_t* freeIndices = nullptr;
        Allocator* allocator = nullptr;

        uint32_t freeIndiceHead = 0;
        uint32_t poolSize = 16;
        uint32_t resourceSize = 4;
        uint32_t usedIndinces = 0;
    };

    template<typename T>
    struct ResourcePoolTyped : public ResourcePool 
    {
        void init(Allocator* alloc, uint32_t poolSize);
        void shutdown();

        T* obtain();
        void release(T* resource);

        T* get(uint32_t index);
        const T* get(uint32_t index) const;
    };
}

#endif // !DATA_STRUCTURE_HDR
