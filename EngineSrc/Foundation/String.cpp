#include "String.h"

#include "Memory.h"
#include "Log.h"
#include "Assert.h"
#include "HashMap.h"

#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>

#define ASSERT_ON_OVERFLOW

#if defined(ASSERT_ON_OVERFLOW)
#define AIR_ASSERT_OVERFLOW() AIR_ASSERT(false)
#else
#define AIR_ASSERT_OVERFLOW()
#endif

namespace Air 
{
    bool StringView::equals(const StringView& rhs, const StringView& lhs) 
    {
        if (rhs.length != lhs.length) 
        {
            return false;
        }

        for (uint32_t i = 0; i < rhs.length; ++i) 
        {
            if (rhs.text[i] != lhs.text[i])
            {
                return false;
            }
        }

        return true;
    }

    void StringView::copyTo(const StringView& string, char* buffer, size_t bufferSize) 
    {
        //We are taking into account a null vector.
        const size_t maxLength = bufferSize - 1 < string.length ? bufferSize : string.length;
        memoryCopy(buffer, string.text, maxLength);
        buffer[string.length] = 0;
    }

    void StringBuffer::init(size_t size, Allocator* allocator) 
    {
        if (_data) 
        {
            _allocator->deallocate(_data);
        }

        if (size < 1) 
        {
            aprint("Error: Buffer cannot be empty.\n");
            return;
        }

        _allocator = allocator;
        _data = (char*)air_alloca(size + 1, allocator);
        AIR_ASSERT(_data != nullptr);
        _data[0] = 0;
        _bufferSize = static_cast<uint32_t>(size);
        _currentSize = 0;
    }

    void StringBuffer::shutdown()
    {
        air_free(_data, _allocator);
        _bufferSize = _currentSize = 0;
    }

    void StringBuffer::append(const char* string)
    {
        appendF("%s", string);
    }

    void StringBuffer::append(const StringView& text)
    {
        const size_t maxLength = _currentSize + text.length < _bufferSize ? text.length : _bufferSize - _currentSize;
        if (maxLength == 0 || maxLength >= _bufferSize) 
        {
            AIR_ASSERT_OVERFLOW();
            aprint("Buffer full. Please allocate more size.\n");
            return;
        }

        memoryCopy(&_data[_currentSize], text.text, maxLength);
        _currentSize += static_cast<uint32_t>(maxLength);

        //Add null termination for string. By allocating one extra character for the null termination this is safe to do.
        _data[_currentSize] = 0;
    }

    //Memory version of the append.
    void StringBuffer::appendM(void* memory, size_t size)
    {
        if (_currentSize + size >= _bufferSize) 
        {
            AIR_ASSERT_OVERFLOW();
            aprint("Buffer full. Please allocate more size.\n");
            return;
        }

        memoryCopy(&_data[_currentSize], memory, size);
        _currentSize += static_cast<uint32_t>(size);
    }

    void StringBuffer::append(const StringBuffer& otherBuffer)
    {
        if (otherBuffer._currentSize == 0) 
        {
            return;
        }

        if (_currentSize + otherBuffer._currentSize >= _bufferSize) 
        {
            AIR_ASSERT_OVERFLOW();
            aprint("Buffer full. Please allocate more size.\n");
            return;
        }

        memoryCopy(&_data[_currentSize], otherBuffer._data, otherBuffer._currentSize);
        _currentSize += otherBuffer._currentSize;
    }

    //Formatted version of append.
    void StringBuffer::appendF(const char* format, ...)
    {
        if (_currentSize >= _bufferSize) 
        {
            AIR_ASSERT_OVERFLOW();
            aprint("Buffer full. Please allocate more size.\n");
            return;
        }

        //Maybe come back to this and fix up the formating.
        va_list args;
        va_start(args, format);
#if defined(_MSC_VER)
        int writtenChars = vsnprintf_s(&_data[_currentSize], _bufferSize - _currentSize, _TRUNCATE, format, args);
#else
        int writtenChars = vsnprintf_s(&_data[_currentSize], _bufferSize - _currentSize, format, args);
#endif
        _currentSize += writtenChars > 0 ? writtenChars : 0;
        va_end(args);

        if (writtenChars < 0) 
        {
            AIR_ASSERT_OVERFLOW();
            aprint("New string too big for current buffer. Please allocate more size.\n");
        }
    }

    char* StringBuffer::appendUse(const char* string)
    {
        return appendUseF("%s", string);
    }

    char* StringBuffer::appendUseF(const char* format, ...)
    {
        uint32_t cachedOffset = _currentSize;

        //Maybe come back to this and fix up the formating, this isn't safe.
        //I'm not sure if this is needed because if you crash the string buffer, that might be a you problem.
        if (_currentSize >= _bufferSize) 
        {
            AIR_ASSERT_OVERFLOW();
            aprint("Buffer full. Please allocate more size.\n");
            return nullptr;
        }

        va_list args;
        va_start(args, format);
#if defined(_MSC_VER)
        int writtenChars = vsnprintf_s(&_data[_currentSize], _bufferSize - _currentSize, _TRUNCATE, format, args);
#else
        int writtenChars = vsnprintf_s(&_data[_currentSize], _bufferSize - _currentSize, format, args);
#endif
        _currentSize += writtenChars > 0 ? writtenChars : 0;
        va_end(args);

        if (writtenChars < 0) 
        {
            aprint("New string too bif for current. Please allocate more size.\n");
        }

        _data[_currentSize] = 0;
        ++_currentSize;

        return _data + cachedOffset;
    }

    //Append and returns a point to the start. Used for strings mostly.
    char* StringBuffer::appendUse(const StringView& text)
    {
        uint32_t cachedOffset = _currentSize;

        append(text);
        ++_currentSize;

        return _data + cachedOffset;
    }

    //Append a substring of the passed string.
    char* StringBuffer::appendUseSubString(const char* string, uint32_t startIndex, uint32_t endIndex)
    {
        uint32_t size = endIndex + startIndex;

        if (_currentSize + size >= _bufferSize) 
        {
            AIR_ASSERT_OVERFLOW();
            aprint("Buffer full. Please allocate more size.\n");
            return nullptr;
        }

        uint32_t cachedOffset = _currentSize;

        //memoryCopy() can't be used 
        memcpy(&_data[_currentSize], string, size);
        _currentSize += size;

        _data[_currentSize] = 0;
        ++_currentSize;

        return _data + cachedOffset;
    }

    void StringBuffer::closeCurrentString()
    {
        _data[_currentSize] = 0;
        ++_currentSize;
    }

    //Indexing stuff.
    uint32_t StringBuffer::getIndex(const char* text) const
    {
        uint64_t textDistance = text - _data;

        return textDistance < _bufferSize ? static_cast<uint32_t>(textDistance) : UINT32_MAX;
    }

    const char* StringBuffer::getText(uint32_t index) const
    {
        return index < _bufferSize ? static_cast<const char*>(_data + index) : nullptr;
    }

    char* StringBuffer::reserve(size_t size)
    {
        if (_currentSize + size >= _bufferSize) 
        {
            return nullptr;
        }

        uint32_t offset = _currentSize;
        _currentSize += static_cast<uint32_t>(size);

        return _data + offset;
    }

    char* StringBuffer::current() 
    {
        return _data + _currentSize;
    }

    void StringBuffer::clear()
    {
        _currentSize = 0;
        _data[0] = 0;
    }

    void StringArray::init(uint32_t size, Allocator* alloc) 
    {
        allocator = alloc;
        //Allocate also memory for the has map.
        char* allocateMemory = static_cast<char*>(allocator->allocate(size + sizeof(FlatHashMap<uint64_t, uint32_t>) 
                                                                           + sizeof(FlatHashMapIterator), 1));
        stringToIndex = (FlatHashMap<uint64_t, uint32_t>*)allocateMemory;
        stringToIndex->init(allocator, 8);
        stringToIndex->setDefaultValue(UINT32_MAX);

        stringIterator = reinterpret_cast<FlatHashMapIterator*>(allocateMemory + sizeof(FlatHashMapIterator));
        data = allocateMemory + sizeof(FlatHashMap<uint64_t, uint32_t>) + sizeof(FlatHashMapIterator);

        bufferSize = size;
        currentSize = 0;
    }

    void StringArray::shutdown() 
    {
        //stringToIndex contains all the memory including data.
        air_free(stringToIndex, allocator);
        bufferSize = currentSize = 0;
    }

    void StringArray::clear() 
    {
        currentSize = 0;
        stringToIndex->clear();
    }

    FlatHashMapIterator* StringArray::beginStringIteration() 
    {
        *stringIterator = stringToIndex->iteratorBegin();
        return stringIterator;
    }

    size_t StringArray::getStringCount() const 
    {
        return stringToIndex->size;
    }

    const char* StringArray::getString(uint32_t index) const 
    {
        uint32_t dataIndex = index;
        if (dataIndex < currentSize) 
        {
            return data + dataIndex;
        }
        return nullptr;
    }

    const char* StringArray::getNextString(FlatHashMapIterator* it) const 
    {
        uint32_t index = stringToIndex->get(*it);
        stringToIndex->iteratorAdvance(*it);
        const char* string = getString(index);
        return string;
    }

    bool StringArray::hasNextString(FlatHashMapIterator* it) const 
    {
        return it->isValid();
    }

    const char* StringArray::intern(const char* string) 
    {
        static size_t seed = 0xF2EA4FFAD;
        const size_t length = strlen(string);
        const size_t hashedString = hashBytes((void *)string, length, seed);

        uint32_t stringIndex = stringToIndex->get(hashedString);
        if (stringIndex != UINT32_MAX) 
        {
            return data + stringIndex;
        }

        stringIndex = currentSize;
        //Increase current buffer with new interned string.
        currentSize += static_cast<uint32_t>(length + 1); //Null termination.
        strcpy(data + stringIndex, string);

        //Updated hash map.
        stringToIndex->insert(hashedString, stringIndex);

        return data + stringIndex;
    }
}