#include "BlobSerialisation.h"

#include "Blob.h"

#include <stdarg.h>
#include <stdio.h>
#include <memory.h>

namespace Air 
{
    //Allocate sie bytes, set the data version start writing.
    //Data version will be saved at the beginning of the file.
    template<typename T>
    T* BlobSerialiser::writeAndPrepare(Allocator* alloc, uint32_t serialiserVersion, size_t size)
    {
        writeCommon(alloc, serialiserVersion, size);

        //Allocate root data. BlobHeader is already allocated in the writeCommon function.
        allocate_static(sizeof(T) - sizeof(BlobHeader));

        //Manually managed blob serilisation.
        dataMemory = nullptr;

        return (T*)blobMemory;
    }

    template<typename T>
    void BlobSerialiser::writeAndSerialise(Allocator* alloc, uint32_t serialiserVersion, size_t size, T* rootData) 
    {
        AIR_ASSERTM(rootData, "Data should never be null.");

        writeCommon(alloc, serialiserVersion, size);

        //Allocate root data. BlobHeader is already allocated in the writeCommon function.
        allocate_static(sizeof(T) - sizeof(BlobHeader));

        //Save root data memory offset calculation
        dataMemory = (char*)rootData;
        //Serilise root data.
        serialise(rootData);
    }

    void BlobSerialiser::writeCommon(Allocator* alloc, uint32_t serialiserVersion, size_t size) 
    {
        allocator = alloc;
        //Allocate memory.
        blobMemory = (char*)air_alloca(size + sizeof(BlobHeader), allocator);
        AIR_ASSERT(blobMemory != nullptr);

        hasAllocatedMemory = 1;

        totalSize = static_cast<uint32_t>(size) + sizeof(BlobHeader);
        serialisedOffset = allocatedOffset = 0;

        this->serialiserVersion = serialiserVersion;
        //This will be written into the blob.
        dataVersion = serialiserVersion;
        isReading = 0;
        isMappable = 0;

        //Write header.
        BlobHeader* header = (BlobHeader*)allocateStatic(sizeof(BlobHeader));
        header->version = serialiserVersion;
        header->mappable = isMappable;

        serialisedOffset = allocatedOffset;
    }

    //Init blob in reading mode from a chunk of perallocated memory. 
    //Size is used to check whether readin is heappening out of the chunk.
    //Allocator is used to allocate memory if needed (for example when reading an array.)
    template<typename T>
    T* BlobSerialiser::read(Allocator* alloc, uint32_t serialiserVersion, size_t size, char* blobMemory, bool forceSerialisation) 
    {
        allocator = alloc;
        this->blobMemory = blobMemory;
        dataMemory = nullptr;

        totalSize = static_cast<uint32_t>(size);

        this->serialiserVersion = serialiserVersion;
        isReading = 1;
        hasAllocatedMemory = 0;

        //Read header from blob.
        BlobHeader* header = (BlobHeader*)blobMemory;
        dataVersion = header->version;
        dataVersion = header->mappable;

        //If serialiser and data are at the same version no need to serialise.
        //TODO: Mappibiliy should be taken is consider.
        if (serialiserVersion == dataVersion && forceSerialisation == false) 
        {
            return (T*)(blobMemory);
        }

        hasAllocatedMemory = 1;
        serialiserVersion = dataVersion;

        //Allocate the data baby.
        dataMemory = (char*)air_allocam(size, allocator);
        T* destinationData = (T*)dataMemory;

        serialisedOffset += sizeof(BlobHeader);

        allocateStatic(sizeof(T));
        //Read from blob to data.
        serialise(destinationData);

        return destinationData;
    }

    void BlobSerialiser::shutdown() 
    {
        if (isReading) 
        {
            //When reading and serialising, we can free blob memory after read.
            //Other we will free pointer when done.
            if (blobMemory && hasAllocatedMemory) 
            {
                air_free(blobMemory, allocator);
            }
        }
        else 
        {
            if (blobMemory) 
            {
                air_free(blobMemory, allocator);
            }
        }

        serialisedOffset = allocatedOffset = 0;
    }

    //This functions are used both for reading and writing.
    //Lead of the serialisation 
    void BlobSerialiser::serialise(char* data)
    {
        if (isReading) 
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(char));
        }
        else 
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(char));
        }

        serialisedOffset += sizeof(char);
    }

    void BlobSerialiser::serialise(int8_t* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(int8_t));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(int8_t));
        }

        serialisedOffset += sizeof(int8_t);
    }

    void BlobSerialiser::serialise(uint8_t* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(uint8_t));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(uint8_t));
        }

        serialisedOffset += sizeof(uint8_t);
    }

    void BlobSerialiser::serialise(int16_t* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(int16_t));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(int16_t));
        }

        serialisedOffset += sizeof(int16_t);
    }

    void BlobSerialiser::serialise(uint16_t* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(uint16_t));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(uint16_t));
        }

        serialisedOffset += sizeof(uint16_t);
    }

    void BlobSerialiser::serialise(int32_t* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(int32_t));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(int32_t));
        }

        serialisedOffset += sizeof(int32_t);
    }

    void BlobSerialiser::serialise(uint32_t* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(uint32_t));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(uint32_t));
        }

        serialisedOffset += sizeof(uint32_t);
    }

    void BlobSerialiser::serialise(int64_t* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(int64_t));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(int64_t));
        }

        serialisedOffset += sizeof(int64_t);
    }

    void BlobSerialiser::serialise(uint64_t* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(uint64_t));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(uint64_t));
        }

        serialisedOffset += sizeof(uint64_t);
    }

    void BlobSerialiser::serialise(float* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(float));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(float));
        }

        serialisedOffset += sizeof(float);
    }

    void BlobSerialiser::serialise(double* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(double));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(double));
        }

        serialisedOffset += sizeof(double);
    }

    void BlobSerialiser::serialise(bool* data)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], sizeof(bool));
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, sizeof(bool));
        }

        serialisedOffset += sizeof(bool);
    }

    void BlobSerialiser::serialise(const char* data)
    {
    }

    template<typename T>
    void BlobSerialiser::serialise(RelativePointer<T>* data)
    {
        if (isReading)
        {
            //Reading: Blob -> Data structure.
            int32_t sourceDataOffset;
            serialise(&sourceDataOffset);

            //Early out to not follow nullptr.
            if (sourceDataOffset == 0)
            {
                data->offset = 0;
                return;
            }
            data->offset = getRelativeDataOffset(data);

            //Allocate memory and set pointer.
            allocateStatic<T>();

            //Cache source serialised offset.
            uint32_t cachedSerialised = serialisedOffset;

            //Move serialisation off. The offset is still this->offset, and the serialise offset.
            //Point just right AFTER it, thus move back by sizeof(offset).
            serialisedOffset = cachedSerialised + sourceDataOffset - sizeof(uint32_t);
            //Serialise/visit the pointed data structure.
            serialise(data->get());
            //Restore serialisation offset.
            serialisedOffset = cachedSerialised;
        }
        else 
        {
            //Writing
            //Data -> blob calculate offset used by RelativePointer.
            //Remember this: char* address = ((char*)&this->offset) + offset;
            //Serialised offset points to what will be the "this->offset"
            //Allocated offset points to the still note allocated memory,
            //Where we will allocate from.
            int32_t dataOffset = allocatedOffset - serialisedOffset;
            serialise(&dataOffset);

            //To jump anywhere and correctly restore the serialisation process,
            //cache the current serialisation offset.
            uint32_t cachedSerialised = serialisedOffset;

            //Move serialisation to thew newly allocated memory at the of the blob.
            serialisedOffset = allocatedOffset;
            //Allocate memory in the blob
            allocateStatic<T>();
            //Serialise/visit the pointed data structure.
            serialise(data->get());
            //Restore serialised
            serialisedOffset = cachedSerialised;
        }
    }

    template<typename T>
    void BlobSerialiser::serialise(RelativeArray<T>* data)
    {
        if (isReading) 
        {
            //Blob --> Data
            serialise(&data->size);

            int32_t sourceDataOffset;
            serialise(&sourceDataOffset);

            //Cache serialised
            uint32_t cachedSerialised = serialisedOffset;

            data->data.offset = getRelativeDataOffset(data) - sizeof(uint32_t);

            //Reserve memory
            allocateStatic(data->size * sizeof(T));

            serialisedOffset = cachedSerialised + sourceDataOffset - sizeof(uint32_t);

            for (uint32_t i = 0; i < data->size; ++i) 
            {
                T* destination = &data->get()[i];
                serialise(destination);
                destination = destination;
            }

            serialisedOffset = cachedSerialised;
        }
        else 
        {
            //Data -> blob
            sierlaise(&data->size);

            //Data will be copied at the end of the current blob.
            int32_t dataOffset = allocatedOffset - serialisedOffset;
            serialise(&dataOffset);

            uint32_t cachedSerialised = serialisedOffset;
            //Move serialisation to the newly allocated memory, at the end of the blob.
            serialisedOffset = allocatedOffset;
            //Allocated memory in the blob.
            allocateStatic(data->size * sizeof(T));

            for (uint32_t i = 0; i < data->size; ++i) 
            {
                T* sourceData = &data->get()[i];
                serialised(sourceData);
                sourceData = sourceData;
            }

            //Restore serialised
            serialisedOffset = cachedSerialised;
        }
    }

    template<typename T>
    void BlobSerialiser::serialise(Array<T>* data)
    {
        if (isReading) 
        {
            //Blob -> data
            serialise(&data->size);

            uint64_t serialisationPad;
            serialise(&serialisationPad);
            serialise(&serialisationPad);

            uint32_t packedDataOffset;
            serialise(&packedDataOffset);
            int32_t sourceDataOffset = static_cast<uint32_t>(packedDataOffset & 0x7FFFFFFF);

            //Cached serialised
            uint32_t cachedSerialised = serialisedOffset;

            data->allocator = nullptr;
            data->capacity = data->size;
            //TODO: Learn why this is commented out.
            //data->relative = (packedDataOffset >> 31);
            //Point string to the end.
            data->data = (T*)(dataMemory + allocatedOffset);
            //data->data.offset = getTraltiveDataOffset(data) - 4;

            //Reserve memory
            allocateStatic(data->size * sizeof(T));

            //sizeof(uint64_t) * 2
            serialisedOffset = cachedSerialised + sourceDataOffset - sizeof(uint32_t);

            for (uint32_t i = 0; i < data->size; ++i) 
            {
                T* destination = &((*data)[i]);
                serialise(destination);
                destination = destination;
            }

            //Restore serialised
            serialisedOffset = cachedSerialised;
        }
        else 
        {
            //Data -> blob
            serialise(&data->size);
            //Add serialisation pads so that we serialise all bytes of the struct Array.
            uint64_t serialisationPad = 0;
            serialise(&serialisationPad);
            serialise(&serialisationPad);

            //Data will be copied at the end of the current blob.
            int32_t dataOffset = allocatedOffset - serialisedOffset;
            //Set higher bit of flag.
            uint32_t packedDataOffset = (static_cast<uint32_t>(dataOffset | (1 << 31)));
            serialise(&packedDataOffset);

            uint32_t cachedSerialised = serialisedOffset;
            //Moved serialisation to the newly allocated memory, at the end of the blob.
            serialisedOffset = allocatedOffset;
            //Allocated memory in the blob
            allocateStatic(data->size * sizeof(T));

            for (uint32_t i = 0; i < data->size; ++i) 
            {
                T* sourceData = &((*data)[i]);
                serialise(sourceData);
                sourceData = sourceData;
            }

            //Restore serialised
            serialisedOffset = cachedSerialised;
        }
    }

    template<typename T>
    void BlobSerialiser::serialise(T* data)
    {
        AIR_ASSERTM(false, "This should never be hit!");
    }

    void BlobSerialiser::serialise(RelativeString* data)
    {
        if (isReading) 
        {
            //Blob -> data
            serialise(&data->size);

            int32_t sourceDataOffset;
            serialise(&sourceDataOffset);

            if (sourceDataOffset > 0) 
            {
                //Cache serialised
                uint32_t cachedSerialised = serialisedOffset;
                serialisedOffset = allocatedOffset;
                data->data.offset = getRelativeDataOffset(data) - 4;

                //Reserve memory + string ending
                allocateStatic(static_cast<size_t>(data->size + 1));

                char* sourceData = blobMemory + cachedSerialised + sourceDataOffset - 4;
                memoryCopy((char*)data->c_str(), sourceData, (size_t)data->size + 1);
                aprint("Found %s\n", data->c_str());
                //Restore serialised
                serialisedOffset = cachedSerialised;
            }
            else 
            {
                data->setEmpty();
            }
        }
        else 
        {
            //Data -> blob
            serialise(&data->size);
            //Data will be copied at the end of the current blob.
            int32_t dataOffset = allocatedOffset - serialisedOffset;
            serialise(&dataOffset);

            uint32_t cachedSerialised = serialisedOffset;
            //Move serialisation to at the end of the blob.
            serialisedOffset = allocatedOffset;
            //Allocate memory in the blob
            allocateStatic(static_cast<size_t>(data->size + 1));

            char* destinationData = blobMemory + serialisedOffset;
            memoryCopy(destinationData, (char*)data->c_str(), static_cast<size_t>(data->size + 1));
            aprint("Written %s, Found %s\n", data->c_str(), destinationData);

            //Restore serilised
            serialisedOffset = cachedSerialised;
        }
    }

    void BlobSerialiser::serialiseMemory(void* data, size_t size)
    {
        if (isReading)
        {
            memoryCopy(data, &blobMemory[serialisedOffset], size);
        }
        else
        {
            memoryCopy(&blobMemory[serialisedOffset], data, size);
        }

        serialisedOffset += static_cast<uint32_t>(size);
    }

    void BlobSerialiser::serialiseMemoryBlock(void** data, uint32_t* size)
    {
        serialise(size);

        if (isReading) 
        {
            //blob -> data
            int32_t sourceDataOffset;
            serialise(&sourceDataOffset);

            if (sourceDataOffset > 0) 
            {
                //Cached serialised
                uint32_t cachedSerialised = serialisedOffset;

                serialisedOffset = allocatedOffset;
                *data = dataMemory + allocatedOffset;

                //Reserve memory
                allocateStatic(*size);

                char* sourceData = blobMemory + cachedSerialised + sourceDataOffset - 4;
                memoryCopy(*data, sourceData, *size);
                //Restore serialised
                serialisedOffset = cachedSerialised;
            }
            else 
            {
                *data = nullptr;
                size = 0;
            }
        }
        else 
        {
            //Data -> Blob
            //Data will be copied at the end of the current blob
            int32_t dataOffset = allocatedOffset - serialisedOffset;
            serialise(&dataOffset);

            uint32_t cachedSerialised = serialisedOffset;
            //Move serialisation to at the end of the blob.
            serialisedOffset = allocatedOffset;
            //Allocated memory in the blob
            allocateStatic(*size);

            char* destinationdata = blobMemory + serialisedOffset;
            memoryCopy(destinationdata, *data, *size);

            //Restore serialised.
            serialisedOffset = cachedSerialised;
        }
    }

    //Static allocation from the blob allocated memory.
    //Just allocates the size of bytes and returns. Used to fill in structures.
    char* BlobSerialiser::allocateStatic(size_t size)
    {
        if (allocatedOffset + size > totalSize) 
        {
            aprint("Blob allocation error: allocated, requested, total - %u + %u > %u", allocatedOffset, size, totalSize);
            return nullptr;
        }

        uint32_t offset = allocatedOffset;
        allocatedOffset += static_cast<uint32_t>(size);

        return isReading ? dataMemory + offset : blobMemory + offset;
    }

    template<typename T>
    T* BlobSerialiser::allocateStatic()
    {
        return (T*)allocateStatic(sizeof(T));
    }

    template<typename T>
    T* BlobSerialiser::allocateAndSet(RelativePointer<T>& data, void* sourceData)
    {
        char* destinationMemory = allocateStatic(sizeof(T));
        data.set(destinationMemory);

        if (sourceData) 
        {
            memoryCopy(destinationMemory, sourceData, sizeof(T));
        }
    }

    //Allocates an array and sets itit so ic can be accessed.
    template<typename T>
    void BlobSerialiser::allocateAndSet(RelativeArray<T>& data, uint32_t numElements, void* sourceData)
    {
        char* destinationMemory = allocateStatic(sizeof(T) * numElements);
        data.set(destinationMemory, numElements);

        if (sourceData)
        {
            memoryCopy(destinationMemory, sourceData, sizeof(T) * numElements);
        }

    }

    //Allocates and sets a static string.
    void BlobSerialiser::allocateAndSet(RelativeString& string, const char* format, ...)
    {
        uint32_t cachedOffset = allocatedOffset;

        char* destinationMemory = isReading ? dataMemory : blobMemory;
        va_list args;
        va_start(args, format);
#if defined(_MSC_VER)
        int writtenChars = vsnprintf_s(&destinationMemory[allocatedOffset], totalSize - allocatedOffset, _TRUNCATE, format, args);
#else
        int writtenChars = vsnprintf_s(&destinationMemory[allocatedOffset], totalSize - allocatedOffset, format, args);
#endif
        allocatedOffset += writtenChars > 0 ? writtenChars : 0;
        va_end(args);

        if (writtenChars < 0) 
        {
            aprint("New string too big for current buffer! Please allocate more size.\n");
        }

        //Add null termination for string.
        //Allocating one extra character for the null termination this is always safe do.
        destinationMemory[allocatedOffset] = 0;
        ++allocatedOffset;

        string.set(destinationMemory + cachedOffset, writtenChars);
    }

    //Allocates and sets a static string.
    void BlobSerialiser::allocateAndSet(RelativeString& string, const char* text, uint32_t length)
    {
        if (allocatedOffset + length > totalSize) 
        {
            aprint("New string too big for current buffer! Please allocate more size.\n");
            return;
        }
        uint32_t cachedOffset = allocatedOffset;

        char* destinationMemory = isReading ? dataMemory : blobMemory;
        memcpy(&destinationMemory[allocatedOffset], text, length);

        allocatedOffset += length;

        //Add null termination for string.
        //Allocating one extra character for the null termination this is always safe do.
        destinationMemory[allocatedOffset] = 0;
        ++allocatedOffset;

        string.set(destinationMemory + cachedOffset, length);
    }

    int32_t BlobSerialiser::getRelativeDataOffset(void* data)
    {
        //dataMemory points to the newly allocated data structure to be used at runtime.
        const int32_t dataOffsetFromStart = static_cast<int32_t>((char*)data - dataMemory);
        const int32_t dataOffset = allocatedOffset - dataOffsetFromStart;
        return dataOffset;
    }
}