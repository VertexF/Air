#ifndef DATA_STRUCTURE_HDR
#define DATA_STRUCTURE_HDR

#include "Memory.h"
#include "Assert.h"

namespace Air 
{
    struct ResourcePool 
    {
        void init(Allocator* alloc, uint32_t poolSize, uint32_t resourceSize);
        void shutdown() const;

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
        void init(Allocator* alloc, uint32_t poolSize)
        {
            ResourcePool::init(alloc, poolSize, sizeof(T));
        }

        void shutdown()
        {
            if (freeIndiceHead != 0)
            {
                aprint("Resource pool has unfreed resources.\n");

                for (uint32_t i = 0; i < freeIndiceHead; ++i)
                {
                    aprint("\tResource %u, %s\n", freeIndices[i], get(freeIndices[i])->name);
                }
            }

            ResourcePool::shutdown();
        }

        T* obtain()
        {
            uint32_t resourceIndex = ResourcePool::obtainResource();
            if (resourceIndex != UINT32_MAX)
            {
                T* resource = get(resourceIndex);
                resource->poolIndex = resourceIndex;
                return resource;
            }
        }

        void release(T* resource)
        {
            ResourcePool::releaseResource(resource->poolIndex);
        }

        T* get(uint32_t index)
        {
            return (T*)ResourcePool::accessResource(index);
        }

        const T* get(uint32_t index) const
        {
            return (const T*)ResourcePool::accessResource(index);
        }
    };
}

#endif // !DATA_STRUCTURE_HDR
