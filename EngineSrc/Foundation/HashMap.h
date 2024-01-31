#ifndef HASH_MAP_HDR
#define HASH_MAP_HDR

#include "Memory.h"
#include "Bit.h"

#if defined(_MSC_VER)
#include <immintrin.h>
#include <intrin0.h>
#endif

#include "vender/wyhash.h"

namespace Air 
{
    template<typename T>
    static uint64_t hashCalculate(const T& value, size_t seed = 0)
    {
        return wyhash(&value, sizeof(T), seed, _wyp);
    }

    template<size_t N>
    static uint64_t hashCalculate(const char(&value)[N], size_t seed = 0)
    {
        return wyhash(value, strlen(value), seed, _wyp);
    }

    static uint64_t hashCalculate(const char*& value, size_t seed)
    {
        return wyhash(value, strlen(value), seed, _wyp);
    }

    static uint64_t hashBytes(void* data, size_t lenght, size_t seed = 0)
    {
        return wyhash(data, lenght, seed, _wyp);
    }

    static const uint64_t ITERATOR_END = UINT64_MAX;

    struct FindInfo 
    {
        uint64_t offset;
        uint64_t probeLength;
    };

    //Simply store the index.
    struct FindResult 
    {
        uint64_t index;
        bool freeIndex;
    };

    //Iterator that stores the index of the entry.
    struct FlatHashMapIterator 
    {
        uint64_t index;
        bool isValid() const { return index != ITERATOR_END; }
        bool isInvalid() const { return index == ITERATOR_END; }
    };

    struct ProbeSequence 
    {
        //This might need to be selectable.
        static const uint64_t WIDTH = 16;
        static const size_t ENGINE_HASH = 0x31D3A36013A;

        ProbeSequence(uint64_t hash, uint64_t mask);

        uint64_t getOffset() const;
        uint64_t getOffset(uint64_t index) const;

        //This is based on a based-0 index.
        uint64_t getIndex() const;

        void next();

        uint64_t mask;
        uint64_t offset;
        uint64_t index = 0;
    };

    template<typename K, typename V>
    struct FlatHashMap
    {
        struct KeyValue
        {
            K key;
            V value;
        };

        void init(Allocator* alloc, uint64_t initialCapacity);
        void shutdown();

        FlatHashMap find(const K& key);
        void insert(const K& key, const V& value);
        uint32_t remove(const K& key);
        uint32_t remove(const FlatHashMapIterator& it);

        V& get(const K& key);
        V& get(const FlatHashMapIterator& it);

        //To difficult to pull out of the header because of a struct with templated types.
        KeyValue& getStructure(const K& key)
        {
            FlatHashMapIterator iterator = find(key);
            if (iterator.index != ITERATOR_END)
            {
                return slots[iterator.index];
            }
            return defaultKeyValue;
        }

        KeyValue& getStructure(const FlatHashMapIterator& it)
        {
            return slots[it.index];
        }

        void setDefaultValue(const V& value);

        //Iterators
        FlatHashMapIterator iteratorBegin();
        void iteratorAdvance(FlatHashMapIterator& iterator);

        void clear();
        void reserve(uint64_t newSize);

        void eraseMeta(const FlatHashMapIterator& iterator);

        FindResult findOrPrepareInsert(const K& key);
        FindInfo findFirstNonFull(uint64_t hash);

        uint64_t prepareInsert(uint64_t hash);

        ProbeSequence probe(uint64_t hash);
        void rehashAndGrowIfNecessary();

        void dropDeletesWithoutResize();
        uint64_t calculateSize(uint64_t newCapacity);

        void initalisedSlots();

        void resize(uint64_t newCapacity);

        void iteratorSkipEmptyOrDeleted(FlatHashMapIterator& iterator);

        //Sets the control byte, and if i < Group::WIDTH - 1, set the cloned byte at the end too.
        void setControl(uint64_t i, int8_t h);
        void resetControl();
        void resetGrowthLeft();

        int8_t* controlBytes = groupInitEmpty();
        KeyValue* slots = nullptr;

        uint64_t size = 0;
        uint64_t capacity = 0;
        uint64_t growthLeft = 0;

        Allocator* allocator = nullptr;
        KeyValue defaultKeyValue = { (K) - 1, 0 };
    };

    struct GroupSse2Impl
    {
        static constexpr size_t WIDTH = 16;
        explicit GroupSse2Impl(const int8_t* pos);

        //Returns a bitmask representing the positions of slots that match hash.
        BitMask<uint32_t, WIDTH> match(int8_t hash) const;

        //Returns a bitmask representing the position of empty slots.
        BitMask<uint32_t, WIDTH> matchEmpty() const;

        BitMask<uint32_t, WIDTH> matchEmptyOrDeleted() const;

        uint32_t countLeadingEmptyOrDeleted() const;

        void convertSpecialToEmptyAndFullToDelete(int8_t* destination) const;

        __m128i control;
    };
}

#endif // !HASH_MAP_HDR
