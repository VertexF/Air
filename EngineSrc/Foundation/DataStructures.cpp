#include "DataStructures.h"

#include <string.h>

namespace Air 
{
    void ResourcePool::init(Allocator* alloc, uint32_t poolSize, uint32_t resourceSize) 
    {
        this->allocator = alloc;
        this->poolSize = poolSize;
        this->resourceSize = resourceSize;

        //Lets groups these together in a sizes of resource + uint32_t
        size_t allocationSize = poolSize * (resourceSize + sizeof(uint32_t));
        memory = (uint8_t*)allocator->allocate(allocationSize, 1);
        memset(memory, 0, allocationSize);

        //Allocate and add free indices
        freeIndices = (uint32_t*)(memory + poolSize * resourceSize);
        freeIndiceHead = 0;

        for (uint32_t i = 0; i < poolSize; ++i) 
        {
            freeIndices[i] = i;
        }

        usedIndinces = 0;
    }

    void ResourcePool::shutdown() const
    {
        if (freeIndiceHead != 0) 
        {
            aprint("Resource pool has unfreed resources.\n");

            for (uint32_t i = 0; i < freeIndiceHead; ++i) 
            {
                aprint("\tResource %u\n", freeIndices[i]);
            }
        }
    }

    uint32_t ResourcePool::obtainResource() 
    {
        //TODO: Try to add bits for checking if resources are alice and use bitmasking to figure that out.
        if (freeIndiceHead < poolSize) 
        {
            const uint32_t freeIndex = freeIndices[freeIndiceHead++];
            ++usedIndinces;
            return freeIndex;
        }

        AIR_ASSERTM(false, "No more resources");
        return UINT32_MAX;
    }

    void ResourcePool::releaseResource(uint32_t index) 
    {
        //TODO: Try to add bits for checking if resources are alice and use bitmasking to figure that out.
        freeIndices[--freeIndiceHead] = index;
        --usedIndinces;
    }

    void ResourcePool::freeAllResources() 
    {
        freeIndiceHead = 0;
        usedIndinces = 0;

        for (uint32_t i = 0; i < poolSize; ++i) 
        {
            freeIndices[i] = i;
        }
    }

    void* ResourcePool::accessResource(uint32_t index) 
    {
        if (index != UINT32_MAX)
        {
            return &memory[index * resourceSize];
        }

        return nullptr;
    }

    const void* ResourcePool::accessResource(uint32_t index) const 
    {
        if (index != UINT32_MAX)
        {
            return &memory[index * resourceSize];
        }

        return nullptr;
    }
}