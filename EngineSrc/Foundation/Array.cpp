#include "Array.h"

namespace Air 
{
    template<typename T>
    Array<T>::Array() 
    {
    }

    template<typename T>
    Array<T>::~Array() 
    {
    }

    template<typename T>
    void Array<T>::init(Allocator* alloc, uint32_t initalCapacity, uint32_t initialSize) 
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

    template<typename T>
    void Array<T>::shutdown() 
    {
        if (capacity > 0) 
        {
            allocator->deallocate(data);
        }

        data = nullptr;
        size = 0;
        capacity = 0;
    }

    template<typename T>
    void Array<T>::push(const T& element) 
    {
        if (size >= capacity) 
        {
            grow(capacity + 1);
        }

        data[size++] = element;
    }

    //Grow the size and return T to be filled.
    template<typename T>
    T& Array<T>::push_use() 
    {
        if (size >= capacity) 
        {
            grow(capacity + 1);
        }
        ++size;

        return back();
    }

    template<typename T>
    void Array<T>::pop() 
    {
        AIR_ASSERT(size > 0);
        --size;
    }

    template<typename T>
    void Array<T>::deleteSwap(uint32_t index) 
    {
        AIR_ASSERT(size > 0 && index < size);
        data[index] = data[--size];
    }

    template<typename T>
    T& Array<T>::operator[](uint32_t index) 
    {
        AIR_ASSERT(index < size);
        return data[index];
    }

    template<typename T>
    const T& Array<T>::operator[](uint32_t index) const 
    {
        AIR_ASSERT(index < size);
        return data[index];
    }

    template<typename T>
    void Array<T>::clear() 
    {
        size = 0;
    }

    template<typename T>
    void Array<T>::setSize(uint32_t newSize) 
    {
        if (newSize > capacity) 
        {
            grow(newSize);
        }
        size = newSize;
    }

    template<typename T>
    void Array<T>::setCapacity(uint32_t newCapacity) 
    {
        if (newCapacity > capacity) 
        {
            grow(newCapacity);
        }
    }

    template<typename T>
    void Array<T>::grow(uint32_t newCapacity) 
    {
        if (newCapacity > capacity * 2)
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

    template<typename T>
    T& Array<T>::back() 
    {
        AIR_ASSERT(size);
        return data[size - 1];
    }

    template<typename T>
    const T& Array<T>::back() const 
    {
        AIR_ASSERT(size);
        return data[size - 1];
    }

    template<typename T>
    T& Array<T>::front() 
    {
        AIR_ASSERT(size);
        return data[0];
    }

    template<typename T>
    const T& Array<T>::front() const 
    {
        AIR_ASSERT(size);
        return data[0];
    }

    template<typename T>
    uint32_t Array<T>::sizeInBytes() const 
    {
        return size * sizeof(T);
    }

    template<typename T>
    uint32_t Array<T>::capacityInByte() const 
    {
        return capacity * sizeof(T);
    }

    template<typename T>
    ArrayView<T>::ArrayView(T* d, uint32_t size) : data(d), size(size)
    {
    }

    template<typename T>
    void ArrayView<T>::set(T* data, uint32_t size) 
    {
        this->data = data;
        this->size = size;
    }

    template<typename T>
    T& ArrayView<T>::operator[](uint32_t index) 
    {
        AIR_ASSERT(index < size);
        return data[index];
    }

    template<typename T>
    const T& ArrayView<T>::operator[](uint32_t index) const
    {
        AIR_ASSERT(index < size);
        return data[index];
    }

}