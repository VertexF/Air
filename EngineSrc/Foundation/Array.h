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
        Array() = default;
        ~Array() = default;

        void init(Allocator* alloc, uint32_t initalCapacity, uint32_t initialSize = 0)
        {
            data = nullptr;
            size = initialSize;
            capacity = 0;
            allocator = alloc;

            if (initalCapacity > 0)
            {
                grow(initalCapacity);
            }
        }

        void shutdown()
        {
            if (capacity > 0)
            {
                allocator->deallocate(data);
            }

            data = nullptr;
            size = 0;
            capacity = 0;
        }

        void push(const T& element)
        {
            if (size >= capacity)
            {
                grow(capacity + 1);
            }

            data[size++] = element;
        }

        //Grow the size and return T to be filled.
        T& push_use()
        {
            if (size >= capacity)
            {
                grow(capacity + 1);
            }
            ++size;

            return back();
        }

        void pop()
        {
            AIR_ASSERT(size > 0);
            --size;
        }

        void deleteSwap(uint32_t index)
        {
            AIR_ASSERT(size > 0 && index < size);
            data[index] = data[--size];
        }

        T& operator[](uint32_t index)
        {
            AIR_ASSERT(index < size);
            return data[index];
        }

        const T& operator[](uint32_t index) const
        {
            AIR_ASSERT(index < size);
            return data[index];
        }

        void clear()
        {
            size = 0;
        }

        void setSize(uint32_t newSize)
        {
            if (newSize > capacity)
            {
                grow(newSize);
            }
            size = newSize;
        }

        void setCapacity(uint32_t newCapacity)
        {
            if (newCapacity > capacity)
            {
                grow(newCapacity);
            }
        }

        void grow(uint32_t newCapacity)
        {
            if (newCapacity < capacity * 2)
            {
                newCapacity = capacity * 2;
            }
            else if (newCapacity < 4)
            {
                newCapacity = 4;
            }

            T* newData = (T*)allocator->allocate(newCapacity * sizeof(T), alignof(T));
            if (capacity)
            {
                memoryCopy(newData, data, capacity * sizeof(T));

                allocator->deallocate(data);
            }

            data = newData;
            capacity = newCapacity;
        }

        T& back()
        {
            AIR_ASSERT(size);
            return data[size - 1];
        }

        const T& back() const
        {
            AIR_ASSERT(size);
            return data[size - 1];
        }

        T& front()
        {
            AIR_ASSERT(size);
            return data[0];
        }

        const T& front() const
        {
            AIR_ASSERT(size);
            return data[0];
        }

        uint32_t sizeInBytes() const
        {
            return size * sizeof(T);
        }

        uint32_t capacityInByte() const
        {
            return capacity * sizeof(T);
        }

        T* data;
        uint32_t size;
        uint32_t capacity;
        Allocator* allocator;
    };

    template<typename T>
    struct ArrayView 
    {

        ArrayView(T* d, uint32_t size) : data(d), size(size)
        {
        }

        void set(T* data, uint32_t size)
        {
            this->data = data;
            this->size = size;
        }

        T& operator[](uint32_t index)
        {
            AIR_ASSERT(index < size);
            return data[index];
        }

        const T& operator[](uint32_t index) const
        {
            AIR_ASSERT(index < size);
            return data[index];
        }

        T* data;
        uint32_t size;
    };
}

#endif // !ARRAY_HDR
