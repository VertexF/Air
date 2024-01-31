#ifndef MEMORY_HDR
#define MEMORY_HDR

#include "Platform.h"
#include "Service.h"

#define AIR_IMGUI

namespace Air 
{
    void memoryCopy(void* destination, void* source, size_t size);
    //Calculate memory alignment size.
    size_t memoryAlign(size_t size, size_t alignment);

    struct MemoryStatistics 
    {
        size_t allocatedBytes;
        size_t totalBytes;

        uint32_t allocationCount;

        void add(size_t a) 
        {
            if (a) 
            {
                allocatedBytes += a;
                ++allocationCount;
            }
        }
    };

    struct Allocator
    {
        virtual ~Allocator() = default;
        virtual void* allocate(size_t size, size_t alignment) = 0;
        virtual void* allocate(size_t size, size_t alignment, const char* file, int32_t line) = 0;

        virtual void deallocate(void * pointer) = 0;
    };

    struct HeapAllocator : public Allocator
    {
        virtual ~HeapAllocator() override = default;

        void init(size_t size);
        void shutdown();

#if defined AIR_IMGUI
        void debugUI();
#endif //AIR_IMGUI

        void* allocate(size_t size, size_t alignment) override;
        void* allocate(size_t size, size_t alignment, const char* file, int32_t line) override;

        void deallocate(void* pointer);

        void* TLSFHandle;
        void* memory;
        size_t allocatedSize = 0;
        size_t maxSize = 0;
    };

    struct StackAllocator : public Allocator
    {
        virtual ~StackAllocator() override = default;

        void init(size_t size, size_t alignment);
        void shutdown();

        void* allocate(size_t size, size_t alignment) override;
        void* allocate(size_t size, size_t alignment, const char* file, int32_t line) override;

        void deallocate(void* pointer) override;

        size_t getMarker();
        void freeMarker(size_t marker);

        void clear();

        uint8_t* memory = nullptr;
        size_t totalSize = 0;
        size_t allocatedSize = 0;
    };

    struct DoubleStackAllocator : public Allocator
    {
        virtual ~DoubleStackAllocator() override = default;

        void init(size_t size);
        void shutdown();

        void* allocate(size_t size, size_t alignment) override;
        void* allocate(size_t size, size_t alignment, const char* file, int32_t line) override;

        void deallocate(void* pointer) override;

        void* allocateTop(size_t size, size_t alignment);
        void* allocateBottom(size_t size, size_t alignment);

        void deallocateTop(size_t size);
        void deallocateBottom(size_t size);

        size_t getTopMarker();
        size_t getBottomMarker();

        void freeTopMarker(size_t marker);
        void freeBottomMarker(size_t marker);

        void clearTop();
        void clearBottom();

        uint8_t* memory = nullptr;
        size_t totalSize = 0;
        size_t top = 0;
        size_t bottom = 0;
    };

    //This allocator can only be reset.
    struct LinearAllocator : public Allocator
    {
        virtual ~LinearAllocator() override = default;

        void init(size_t size);
        void shutdown();

        void* allocate(size_t size, size_t alignment) override;
        void* allocate(size_t size, size_t alignment, const char* file, int32_t line) override;

        void deallocate(void* pointer) override;

        void clear();

        uint8_t* memory = nullptr;
        size_t totalSize = 0;
        size_t allocatedSize = 0;
    };

    //DO NOT use this for runtime processes. ONLY compilation resources.
    //Don't use to allocate stuff in run time.
    struct MallocAllocator : public Allocator
    {
        void* allocate(size_t size, size_t alignment) override;
        void* allocate(size_t size, size_t alignment, const char* file, int32_t line) override;

        void deallocate(void* pointer) override;
    };

    struct MemoryServiceConfiguration 
    {
        //We can only allocate 32MB of memory all at once by default.
        size_t maximumDynamicSize = 32 * 1024 * 1024;
    };

    struct MemoryService : public Service 
    {
        virtual ~MemoryService() = default;

        AIR_DECLARE_SERVICE(MemoryService);

        void init(void* configuration);
        void shutdown();

#if defined AIR_IMGUI
        void imguiDraw();
#endif

        LinearAllocator scratchAllocator;
        HeapAllocator systemAllocator;

        //tests the allocators.
        void test();

        static constexpr const char* name = "Air Memory Service";
    };

#define air_alloca(size, allocator) ((allocator)->allocate(size, 1, __FILE__, __LINE__))
#define air_allocam(size, allocator) ((uint8_t*)(allocator)->allocate(size, 1, __FILE__, __LINE__))
#define air_allocat(type, allocator) ((type*)(allocator)->allocate(sizeof(type), 1, __FILE__, __LINE__))

#define air_allocaa(size, allocator, alignment) ((allocator)->allocate(size, alignment, __FILE__, __LINE__))

#define air_free(pointer, allocate) ((allocate)->deallocate(pointer))

#define air_kilo(size) (size * 1024)
#define air_mega(size) (size * 1024 * 1024)
#define air_giga(size) (size * 1024 * 1024 * 1024)
}

#endif // !MEMORY_HDR
