#ifndef ARRAY_HDR
#define ARRAY_HDR

#include "Memory.h"
#include "Assert.h"

namespace Air 
{
    //AlignedArray
    template<typename T>
    struct Array 
    {
        Array();
        ~Array();

        void init(Allocator* alloc, uint32_t initalCapacity, uint32_t initialSize = 0);
        void shutdown();

        void push(const T& element);
        //Grow the size and return T to be filled.
        T& push_use();

        void pop();
        void deleteSwap(uint32_t index);

        T& operator[](uint32_t index);
        const T& operator[](uint32_t index) const;

        void clear();
        void setSize(uint32_t newSize);
        void setCapacity(uint32_t newCapacity);
        void grow(uint32_t newCapacity);

        T& back();
        const T& back() const;

        T& front();
        const T& front() const;

        uint32_t sizeInBytes() const;
        uint32_t capacityInByte() const;

        T* data;
        uint32_t size;
        uint32_t capacity;
        Allocator* allocator;
    };

    template<typename T>
    struct ArrayView 
    {
        ArrayView(T* d, uint32_t size);

        void set(T* data, uint32_t size);

        T& operator[](uint32_t index);
        const T& operator[](uint32_t index) const;

        T* data;
        uint32_t size;
    };
}

#endif // !ARRAY_HDR
