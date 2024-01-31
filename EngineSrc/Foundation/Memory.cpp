#include "Memory.h"
#include "MemoryUtils.h"
#include "Assert.h"

#include <vender/tlsf.h>

#include <stdlib.h>
#include <memory.h>

#if defined AIR_IMGUI
    #include <vender/imgui/imgui.h>
#endif

//Define this and add StackWalker to heavy memory profile.
//#define AIR_MEMORY_STACK

#define HEAP_ALLOCATOR_STATS

#if defined(AIR_MEMORY_STACK)
    #include <vender/StackWalker.h>
#endif //AIR_MEMORY_STACK

namespace Air
{

    //#define AIR_MEMORY_DEBUG
#if defined(AIR_MEMORY_DEBUG)
#define AIR_MEM_ASSERT(condition, message, ...) AIR_ASSERTM(condition, message, __VA_ARGS__)
#else
#define AIR_MEM_ASSERT(condition, message, ...)
#endif //AIR_MEMORY_DEBUG

    static MemoryService MEMORY_SERVICE;

    static size_t SIZE = air_mega(32) + tlsf_size() + 8;

    static void exitWalker(void* ptr, size_t size, int used, void* user);
    static void imguiWalker(void* ptr, size_t size, int used, void* user);

    MemoryService* MemoryService::instance()
    {
        return &MEMORY_SERVICE;
    }

    void MemoryService::init(void* configuration)
    {
        aprint("Memory Service Init.\n");
        MemoryServiceConfiguration* memoryConfiguration = static_cast<MemoryServiceConfiguration*>(configuration);
        systemAllocator.init(memoryConfiguration ? memoryConfiguration->maximumDynamicSize : SIZE);
    }

    void MemoryService::shutdown()
    {
        systemAllocator.shutdown();

        aprint("Memory Service Shutdown.\n");
    }

    void exitWalker(void* ptr, size_t size, int used, void* user)
    {
        MemoryStatistics* stats = (MemoryStatistics*)user;
        stats->add(used ? size : 0);

        if (used)
        {
            aprint("Found active allocation %p %llu\n", ptr, size);
        }
    }

#if defined(AIR_IMGUI)
    void imguiWalker(void* ptr, size_t size, int used, void* user)
    {
        uint32_t memorySize = (uint32_t)size;
        //Memory units as in megabytes or petabytes
        const char* memoryUnit = "b";
        if (memorySize > 1024 * 1024)
        {
            memorySize /= 1024 * 1024;
            memoryUnit = "Mb";
        }
        else if (memorySize > 1024)
        {
            memorySize /= 1024;
            memoryUnit = "Kb";
        }
        ImGui::Text("\t%p %s size: %4llu %s\n", ptr, used ? "used" : "free", memorySize, memoryUnit);

        MemoryStatistics* stats = (MemoryStatistics*)user;
        stats->add(used ? size : 0);
    }

    void MemoryService::imguiDraw()
    {
        if (ImGui::Begin("Memory Service"))
        {
            systemAllocator.debugUI();
        }
        ImGui::End();
    }
#endif //AIR_IMGUI

    void MemoryService::test()
    {
        //TODO Write and test the memory allocators and fix what the API is vs what's in the code.
        //static LinearAllocator linear;
        //linear.init(1024);
    }

    void HeapAllocator::init(size_t size)
    {
        memory = malloc(size);
        maxSize = size;
        allocatedSize = 0;

        TLSFHandle = tlsf_create_with_pool(memory, size);
        aprint("HeapAllocator of size %llu created.\n", size);
    }

    void HeapAllocator::shutdown()
    {
        MemoryStatistics stats{ 0, maxSize };
        pool_t pool = tlsf_get_pool(TLSFHandle);
        tlsf_walk_pool(pool, exitWalker, (void*)&stats);

        if (stats.allocatedBytes)
        {
            aprint("HeapAllocator Shutdown.\n=========\nFAILURE! Allocated memory detected. Allocated %llu, total %llu\n=========\n", stats.allocatedBytes, stats.totalBytes);
        }
        else
        {
            aprint("HeapAllocator Shutdown - all memory freed.\n");
        }

        AIR_ASSERTM(stats.allocatedBytes == 0, "Allocated bytes are still present.");

        tlsf_destroy(TLSFHandle);

        free(memory);
    }

#if defined AIR_IMGUI
    void HeapAllocator::debugUI()
    {
        ImGui::Separator();
        ImGui::Text("Heap Allocator");
        ImGui::Separator();
        MemoryStatistics stats{ 0, maxSize };
        pool_t pool = tlsf_get_pool(TLSFHandle);
        tlsf_walk_pool(pool, imguiWalker, (void*)&stats);

        ImGui::Separator();
        ImGui::Text("\tAllocation count %d", stats.allocationCount);
        ImGui::Text("\tAllocated %llu K, free %llu Mb", stats.allocatedBytes / (1024 * 1024),
            maxSize - stats.allocatedBytes / (1024 * 1024),
            maxSize / (1024 * 1024));
    }
#endif //AIR_IMGUI

#if defined (AIR_MEMORY_STACK)
    class AirStackWalker : public StackWalker
    {
    public:
        AirStackWalker() : StackWalker()
        {
        }

    protected:
        virtual void OnOuput(LPCSTR szText)
        {
            aprint("\nStackL \n%s\n", szText);
            StackWalker::OnOutput(szText);
        }
    };

    void* HeapAllocator::allocate(size_t size, size_t alignment)
    {
        void* memory = tlsf_malloc(TLSFHandle, size);
        aprint("Memory: %p, size %llu \n", memory, size);
        return memory;
    }
#else
    void* HeapAllocator::allocate(size_t size, size_t alignment)
    {
#if defined(HEAP_ALLOCATOR_STATS)
        void* allocatedMemory = alignment == 1 ? tlsf_malloc(TLSFHandle, size) : tlsf_memalign(TLSFHandle, alignment, size);
        size_t actualSize = tlsf_block_size(allocatedMemory);
        allocatedSize += actualSize;

        return allocatedMemory;
#else
        return tlsf_malloc(TLSFHandle, size);
#endif
    }
#endif //AIR_MEMORY_STACK

    void* HeapAllocator::allocate(size_t size, size_t alignment, const char* file, int32_t line)
    {
        return allocate(size, alignment);
    }

    void HeapAllocator::deallocate(void* pointer)
    {
#if defined (HEAP_ALLOCATOR_STATS)
        size_t actualSize = tlsf_block_size(pointer);
        allocatedSize -= actualSize;

        tlsf_free(TLSFHandle, pointer);
#else
        tlsf_free(TLSFHandle, pointer);
#endif
    }

    void LinearAllocator::init(size_t size)
    {
        memory = (uint8_t*)malloc(size);
        totalSize = size;
        allocatedSize = 0;
    }

    void LinearAllocator::shutdown()
    {
        clear();
        free(memory);
    }

    void* LinearAllocator::allocate(size_t size, size_t alignment)
    {
        AIR_ASSERT(size > 0);

        const size_t newStart = memoryAlign(allocatedSize, alignment);
        AIR_ASSERT(newStart < totalSize);
        const size_t newAllocatedSize = newStart + size;
        if (newAllocatedSize > totalSize)
        {
            AIR_MEM_ASSERT(false, "Overflow");
            return nullptr;
        }

        allocatedSize = newAllocatedSize;
        return memory + newStart;
    }

    void* LinearAllocator::allocate(size_t size, size_t alignment, const char* file, int32_t line)
    {
        return allocate(size, alignment);
    }

    void LinearAllocator::deallocate(void* /*pointer*/)
    {
        //This allocator does not allocate on a per pointer bases.
    }

    void LinearAllocator::clear() 
    {
        allocatedSize = 0;
    }

    void memoryCopy(void* destination, void* source, size_t size) 
    {
        memcpy(destination, source, size);
    }

    size_t memoryAlign(size_t size, size_t alignment) 
    {
        const size_t alignmentMask = alignment - 1;
        return (size + alignmentMask) & ~alignmentMask;
    }

    void* MallocAllocator::allocate(size_t size, size_t alignment) 
    {
        return malloc(size);
    }

    void* MallocAllocator::allocate(size_t size, size_t alignment, const char* file, int32_t line) 
    {
        return malloc(size);
    }

    void MallocAllocator::deallocate(void* pointer) 
    {
        free(pointer);
    }

    void StackAllocator::init(size_t size, size_t alignment) 
    {
        memory = (uint8_t*)malloc(size);
        allocatedSize = 0;
        totalSize = size;
    }

    void StackAllocator::shutdown() 
    {
        free(memory);
    }

    void* StackAllocator::allocate(size_t size, size_t alignment) 
    {
        AIR_ASSERT(size > 0);

        const size_t newStart = memoryAlign(allocatedSize, alignment);
        AIR_ASSERT(newStart < totalSize);

        const size_t newAllocatedSize = newStart + size;
        if (newAllocatedSize > totalSize)
        {
            AIR_MEM_ASSERT(false, "Overflow");
            return nullptr;
        }

        allocatedSize = newAllocatedSize;
        return memory + newStart;
    }

    void* StackAllocator::allocate(size_t size, size_t alignment, const char* file, int32_t line) 
    {
        return allocate(size, alignment);
    }

    void StackAllocator::deallocate(void* pointer) 
    {
        AIR_ASSERT(pointer >= memory);
        AIR_ASSERTM(pointer < memory + totalSize, "Out of bound free on linear allocator (outside bounds). Try a tempting to free %p, %llu after beginning of buffer (memory %p size %llu, allocated %llu)", (uint8_t*)pointer, (uint8_t*)pointer - memory, memory, totalSize, allocatedSize);
        AIR_ASSERTM(pointer < memory + allocatedSize, "Out of bound free on linear allocator (inside bounds, after allocated). Try a tempting to free %p, %llu after beginning of buffer (memory %p size %llu, allocated %llu)", (uint8_t*)pointer, (uint8_t*)pointer - memory, memory, totalSize, allocatedSize);

        const size_t sizeAtPointer = (uint8_t *)pointer - memory;
        allocatedSize = sizeAtPointer;
    }

    size_t StackAllocator::getMarker() 
    {
        return allocatedSize;
    }

    void StackAllocator::freeMarker(size_t marker) 
    {
        const size_t difference = marker - allocatedSize;
        if (difference > 0) 
        {
            allocatedSize = marker;
        }
    }

    void StackAllocator::clear() 
    {
        allocatedSize = 0;
    }

    void DoubleStackAllocator::init(size_t size) 
    {
        memory = (uint8_t*)malloc(size);
        top = size;
        bottom = 0;
        totalSize = size;
    }

    void DoubleStackAllocator::shutdown() 
    {
        free(memory);
    }

    void* DoubleStackAllocator::allocate(size_t size, size_t alignment) 
    {
        AIR_ASSERTM(false, "You can't do that with a double stack allocator.");
        return nullptr;
    }

    void* DoubleStackAllocator::allocate(size_t size, size_t alignment, const char* file, int32_t line) 
    {
        AIR_ASSERTM(false, "You can't do that with a double stack allocator.");
        return nullptr;
    }

    void DoubleStackAllocator::deallocate(void* pointer) 
    {
        AIR_ASSERTM(false, "You can't do that with a double stack allocator.");
    }

    void* DoubleStackAllocator::allocateTop(size_t size, size_t alignment) 
    {
        AIR_ASSERT(size > 0);

        const size_t newStart = memoryAlign(top - size, alignment);
        if (newStart <= bottom) 
        {
            AIR_MEM_ASSERT(false, "Overflow Crossing");
            return nullptr;
        }

        top = newStart;
        return memory + newStart;
    }

    void* DoubleStackAllocator::allocateBottom(size_t size, size_t alignment) 
    {
        AIR_ASSERT(size > 0);

        const size_t newStart = memoryAlign(bottom, alignment);
        const size_t newAllocatedSize = newStart + size;
        if (newAllocatedSize >= top) 
        {
            AIR_MEM_ASSERT(false, "Overflow Crossing");
            return nullptr;
        }

        bottom = newAllocatedSize;
        return memory + newStart;
    }

    void DoubleStackAllocator::deallocateTop(size_t size) 
    {
        if (size > totalSize - top) 
        {
            top = totalSize;
        }
        else 
        {
            top += size;
        }
    }

    void DoubleStackAllocator::deallocateBottom(size_t size) 
    {
        if (size > bottom)
        {
            bottom = 0;
        }
        else
        {
            bottom -= size;
        }
    }

    size_t DoubleStackAllocator::getTopMarker() 
    {
        return top;
    }

    size_t DoubleStackAllocator::getBottomMarker() 
    {
        return bottom;
    }

    void DoubleStackAllocator::freeTopMarker(size_t marker) 
    {
        if (marker > top && marker < totalSize) 
        {
            top = marker;
        }
    }

    void DoubleStackAllocator::freeBottomMarker(size_t marker) 
    {
        if (marker < bottom) 
        {
            bottom = marker;
        }
    }

    void DoubleStackAllocator::clearTop() 
    {
        top = totalSize;
    }

    void DoubleStackAllocator::clearBottom() 
    {
        bottom = 0;
    }

}