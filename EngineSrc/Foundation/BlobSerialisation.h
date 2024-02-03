#ifndef BLOB_SERIALISATION_HDR
#define BLOB_SERIALISATION_HDR

#include "Platform.h"
#include "RelativeDataStructures.h"
#include "Array.h"

namespace Air 
{
    struct Allocator;

    struct BlobSerialiser 
    {
        //Allocate sie bytes, set the data version start writing.
        //Data version will be saved at the beginning of the file.
        template<typename T>
        T* writeAndPrepare(Allocator* alloc, uint32_t serialiseVersion, size_t size);

        template<typename T>
        void writeAndSerialise(Allocator* alloc, uint32_t serialisVersion, size_t size, T* rootData);

        void writeCommon(Allocator* alloc, uint32_t serialiserVersion, size_t size);

        //Init blob in reading mode from a chunk of perallocated memory. 
        //Size is used to check whether readin is heappening out of the chunk.
        //Allocator is used to allocate memory if needed (for example when reading an array.)
        template<typename T>
        T* read(Allocator* alloc, uint32_t serialiserVersion, size_t size, char* blobMemory, bool forceSerialisation = false);
        void shutdown();

        //This functions are used both for reading and writing.
        //Lead of the serialisation 
        void serialise(char* data);
        void serialise(int8_t* data);
        void serialise(uint8_t* data);
        void serialise(int16_t* data);
        void serialise(uint16_t* data);
        void serialise(int32_t* data);
        void serialise(uint32_t* data);
        void serialise(int64_t* data);
        void serialise(uint64_t* data);
        void serialise(float* data);
        void serialise(double* data);
        void serialise(bool* data);
        void serialise(const char* data);

        template<typename T>
        void serialise(RelativePointer<T>* data);

        template<typename T>
        void serialise(RelativeArray<T>* data);

        template<typename T>
        void serialise(Array<T>* data);

        template<typename T>
        void serialise(T* data);

        void serialise(RelativeString* data);

        void serialiseMemory(void* data, size_t size);
        void serialiseMemoryBlock(void** data, uint32_t* size);

        //Static allocation from the blob allocated memory.
        //Just allocates the size of bytes and returns. Used to fill in structures.
        char* allocateStatic(size_t size);

        template<typename T>
        T* allocateStatic();

        template<typename T>
        T* allocateAndSet(RelativePointer<T>& data, void* sourceData = nullptr);

        //Allocates an array and sets itit so ic can be accessed.
        template<typename T>
        void allocateAndSet(RelativeArray<T>& data, uint32_t numElements, void* sourceData = nullptr);

        //Allocates and sets a static string.
        void allocateAndSet(RelativeString& string, const char* format, ...);
        //Allocates and sets a static string.
        void allocateAndSet(RelativeString& string, const char* text, uint32_t length);

        int32_t getRelativeDataOffset(void* data);

        char* blobMemory = nullptr;
        char* dataMemory = nullptr;

        Allocator* allocator = nullptr;

        uint32_t totalSize = 0;
        uint32_t serialisedOffset = 0;
        uint32_t allocatedOffset = 0;

        //Version coming from the code.
        uint32_t serialiserVersion = UINT32_MAX;
        //Version read from blob or written into blob
        uint32_t dataVersion = UINT32_MAX;

        uint32_t isReading = 0;
        uint32_t isMappable = 0;

        uint32_t hasAllocatedMemory = 0;
    };
}

#endif // !BLOB_SERIALISATION_HDR
