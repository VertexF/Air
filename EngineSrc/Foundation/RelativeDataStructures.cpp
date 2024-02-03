#include "RelativeDataStructures.h"

#include "Memory.h"
#include "Assert.h"
#include "Array.h"

namespace Air 
{
    template<typename T>
    T* RelativePointer<T>::get() const 
    {
        char* address = ((char*)&offset) + offset;
        return offset != 0 ? (T*)address : nullptr;
    }

    template<typename T>
    bool RelativePointer<T>::isEqual(const RelativePointer& other) const 
    {
        return get() == other.get();
    }

    template<typename T>
    bool RelativePointer<T>::isNull() const 
    {
        return offset == 0;
    }

    template<typename T>
    bool RelativePointer<T>::isNotNull() const 
    {
        return offset != 0;
    }

    //This hopefully gives us a cleaner interface.
    template<typename T>
    T* RelativePointer<T>::operator->() const 
    {
        return get();
    }

    template<typename T>
    T& RelativePointer<T>::operator*() const 
    {
        return *(get());
    }

#if defined(AIR_BLOB_WRITE)
    template<typename T>
    void RelativePointer<T>::set(char* rawPointer) 
    {
        offset = rawPointer ? (int32_t)(rawPointer - (char*)this) : 0;
    }

    template<typename T>
    void RelativePointer<T>::setNull() 
    {
        offset = 0;
    }
#endif

    template <typename T>
    const T& RelativeArray<T>::operator[](uint32_t index) const 
    {
        AIR_ASSERT(index < size);
        return data.get()[index]; 
    }

    template <typename T>
    T& RelativeArray<T>::operator[](uint32_t index) 
    {
        AIR_ASSERT(index < size);
        return data.get()[index];
    }

    template <typename T>
    const T* RelativeArray<T>::get() const 
    {
        return data.get();
    }

    template <typename T>
    T* RelativeArray<T>::get() 
    {
        return data.get();
    }

#if defined(AIR_BLOB_WRITE)
    template <typename T>
    void RelativeArray<T>::set(char* rawPointer, uint32_t size) 
    {
        data.set(rawPointer);
        this->size = size;
    }

    template <typename T>
    void RelativeArray<T>::setEmpty() 
    {
        size = 0;
        data.setNull();
    }
#endif
}