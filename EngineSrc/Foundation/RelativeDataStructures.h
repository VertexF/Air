#ifndef RELATIVE_DATA_STRUCTURES_HDR
#define RELATIVE_DATA_STRUCTURES_HDR

#include "Platform.h"

//This is used in the code to write blueprints, like data compilers
#define AIR_BLOB_WRITE

namespace Air 
{
    struct Allocator;

    template<typename T>
    struct RelativePointer
    {
        T* get() const;

        bool isEqual(const RelativePointer& other) const;
        bool isNull() const;
        bool isNotNull() const;

        //This hopefully gives us a cleaner interface.
        T* operator->() const;
        T& operator*() const;

#if defined(AIR_BLOB_WRITE)
        void set(char* rawPointer);
        void setNull();
#endif

        int32_t offset = 0;
    };

    template <typename T>
    struct RelativeArray 
    {
        const T& operator[](uint32_t index) const;
        T& operator[](uint32_t index);

        const T* get() const;
        T* get();

#if defined(AIR_BLOB_WRITE)
        void set(char* rawPointer, uint32_t size);
        void setEmpty();
#endif

        uint32_t size = 0;
        RelativePointer<T> data;
    };

    struct RelativeString : public RelativeArray<char> 
    {
        const char* c_str() const { return data.get(); }
        void set(char* ptr, uint32_t size) { return RelativeArray<char>::set(ptr, size); }
    };
}

#endif // !RELATIVE_DATA_STRUCTURES_HDR
