#include "HashMap.h"

#include "Assert.h"

#if defined(_MSC_VER)
    //  Windows
    #include <Windows.h>
    #define WIN32_LEAN_AND_MEAN
    static bool SSSE3_SUPPORT = IsProcessorFeaturePresent(PF_SSSE3_INSTRUCTIONS_AVAILABLE);
#else
    //  GCC Intrinsics
    #include <cpuid.h>
    static void cpuid(int info[4], int InfoType) 
    {
        __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
    }

    int info[4];
    cpuid(info, 0);
    int nIds = info[0];

    cpuid(info, 0x80000000);
    unsigned nExIds = info[0];

    static bool SSSE3_SUPPORT = (info[2] & ((int)1 << 9)) != 0;
#endif

namespace Air 
{
    //Control bytes
    //This is following Google's abseil librar convention - based on performance.
    static const int8_t CONTROL_BITMASK_EMPTY = -128;  //0b10000000;
    static const int8_t CONTROL_BITMASK_DELETED = -2;  //0b11111110;
    static const int8_t CONTROL_BITMASK_SENTINEL = -1; //0b11111111;

    static bool controlIsEmpty(int8_t control) { return control == CONTROL_BITMASK_EMPTY; }
    static bool controlIsFull(int8_t control) { return control >= 0; }
    static bool controlIsDeleted(int8_t control) { return control == CONTROL_BITMASK_DELETED; }
    static bool controlIsEmptyOrDeleted(int8_t control) { return control < CONTROL_BITMASK_SENTINEL; }

    //All this below returns a hash seed
    //This seed consists of the ctrl_pointer, which adds enough entropy to ensure non-determinisim of iterator order in most case.
    //Implementations details: the low bits of the pointer have little or no entropy because of alignment. 
    //We shift the pointer to try to use higher entry bits. 
    //A good number seems to be 12 bits, because that aligns with the page size.

    static uint64_t hashSeed(const int8_t* control) { return reinterpret_cast<uintptr_t>(control) >> 12; }

    static uint64_t hash1(uint64_t hash, const int8_t* control) { return (hash >> 7) ^ hashSeed(control); }
    static int8_t hash2(uint64_t hash) { return hash & 0x7F; }

    static bool capacityIsValid(size_t n) { return ((n + 1) & n) == 0 && n > 0; }

    static uint64_t lzcntSoft(uint64_t n) 
    {
        //Note(macro): __lzcnt intrisics require at least aswell.
#if defined(_MSC_VER)
        unsigned long index = 0;
        _BitScanReverse64(&index, n);
        uint64_t cnt = index ^ 63;
#else
        uint64_t cnt = __builtin_clzl(n);
#endif
        return cnt;
    }

    //Rounds up the apacity to the next power of 2 minus 1 with a minimum of 1.
    static uint64_t capacityNormalise(uint64_t n) { return n ? ~uint64_t{} >> lzcntSoft(n) : 1; }

    //General notes of capacity/growth methods below:
    //We use 7/8th as a maximum load factor. For 16-wide groups, that gives an average of two empty slots per group.
    //For (capacity + 1) >= Group::WIDTH, growth 7/8 * capacity.
    //For (capacity + 1) <  Group::WIFTH, growth == capacity. - In this case, we never need to probe (the whole table fits in one group)
    //so we don't need a load factor less than 1

    //Give 'capacity' of the table, returns the size (i.e. number of full slots) at which we should grow the capacity.
    //if (Group::WIDTH == 8 && capacity == 7) { return 6; }
    //x - x / 8 does not work when x == 7
    static uint64_t capacityToGrowth(uint64_t capacity) { return capacity - capacity / 8; }
    static uint64_t capacityGrowthToLowerBound(uint64_t growth) { return growth + static_cast<uint64_t>((static_cast<int64_t>(growth) - 1) / 7); }

    static void convertDeletedToEmptyAndFullToDeleted(int8_t* control, size_t capacity) 
    {
        for (int8_t* pos = control; pos != control + capacity + 1; pos += GroupSse2Impl::WIDTH)
        {
            GroupSse2Impl{ pos }.convertSpecialToEmptyAndFullToDelete(pos);
        }

        memoryCopy(control + capacity + 1, control, GroupSse2Impl::WIDTH);
        control[capacity] = CONTROL_BITMASK_SENTINEL;
    }

    static int8_t* groupInitEmpty() 
    {
        alignas(16) static constexpr int8_t emptyGroup[] =
        {
            CONTROL_BITMASK_SENTINEL, 
            CONTROL_BITMASK_EMPTY, CONTROL_BITMASK_EMPTY, CONTROL_BITMASK_EMPTY,
            CONTROL_BITMASK_EMPTY, CONTROL_BITMASK_EMPTY, CONTROL_BITMASK_EMPTY,
            CONTROL_BITMASK_EMPTY, CONTROL_BITMASK_EMPTY, CONTROL_BITMASK_EMPTY,
            CONTROL_BITMASK_EMPTY, CONTROL_BITMASK_EMPTY, CONTROL_BITMASK_EMPTY,
            CONTROL_BITMASK_EMPTY, CONTROL_BITMASK_EMPTY, CONTROL_BITMASK_EMPTY
        };

        return const_cast<int8_t*>(emptyGroup);
    }

    ProbeSequence::ProbeSequence(uint64_t hash, uint64_t mask) : 
        mask(mask), offset(hash & mask)
    {
    }

    uint64_t ProbeSequence::getOffset() const 
    {
        return offset;
    }

    uint64_t ProbeSequence::getOffset(uint64_t index) const 
    {
        return (offset + index) & mask;
    }

    //This is based on a based-0 index.
    uint64_t ProbeSequence::getIndex() const 
    {
        return index;
    }

    void ProbeSequence::next() 
    {
        index += WIDTH;
        offset += index;
        offset &= mask;
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::init(Allocator* alloc, uint64_t initialCapacity)
    {
        allocator = alloc;
        size = capacity = growthLeft = 0;
        defaultKeyValue = { (K) - 1, (V)0 };

        controlBytes = groupInitEmpty();
        slots = nullptr;
        reserve(initialCapacity < 4 ? 4 : initialCapacity);
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::shutdown() 
    {
        air_free(controlBytes, allocator);
    }

    template<typename K, typename V>
    FlatHashMap<K, V> FlatHashMap<K, V>::find(const K& key)
    {
        const uint64_t hash = hashCalculate(key);
        ProbeSequence sequence = probe(hash);

        while(true)
        {
            const GroupSse2Impl group{ controlBytes + sequence.getOffset() };
            const int8_t hash2 = hash2(hash);
            for (int i : group.match(hash2)) 
            {
                const KeyValue& keyValue = *(slots + sequence.getOffset(i));
                if (keyValue.key == key) 
                {
                    return { sequence.getOffset(i) };
                }
            }

            if (group.matchEmpty()) 
            {
                break;
            }

            sequence.next();
        }

        return { ITERATOR_END };
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::insert(const K& key, const V& value) 
    {
        const FindResult findResult = findOrPrepareInsert(key);
        if (findResult.freeIndex) 
        {
            //Emplace
            slots[findResult.index].key = key;
            slots[findResult.index].value = value;
        }
        else 
        {
            //Substitute value index
            slots[findResult.index].value = value;
        }
    }

    template<typename K, typename V>
    uint32_t FlatHashMap<K, V>::remove(const K& key) 
    {
        FlatHashMapIterator iterator = find(key);
        if (iterator.index == ITERATOR_END) 
        {
            return 0;
        }

        eraseMeta(iterator);
        return 1;
    }

    template<typename K, typename V>
    uint32_t FlatHashMap<K, V>::remove(const FlatHashMapIterator& it) 
    {
        if (it.index == ITERATOR_END) 
        {
            return 0;
        }
        eraseMeta(it);
        return 1;
    }

    template<typename K, typename V>
    V& FlatHashMap<K, V>::get(const K& key) 
    {
        FlatHashMapIterator iterator = find(key);
        if (iterator.index != ITERATOR_END) 
        {
            return slots[iterator.index].value;
        }

        return defaultKeyValue.value;
    }

    template<typename K, typename V>
    V& FlatHashMap<K, V>::get(const FlatHashMapIterator& it) 
    {
        if (it.index != ITERATOR_END) 
        {
            return slots[it.index].value;
        }

        return defaultKeyValue.value;
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::setDefaultValue(const V& value) 
    {
        defaultKeyValue = value;
    }

    //Iterators
    template<typename K, typename V>
    FlatHashMapIterator FlatHashMap<K, V>::iteratorBegin() 
    {
        FlatHashMapIterator it{ 0 };
        iteratorSkipEmptyOrDeleted(it);
        return it;
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::iteratorAdvance(FlatHashMapIterator& iterator) 
    {
        iterator.index++;
        iteratorSkipEmptyOrDeleted(iterator);
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::clear() 
    {
        size = 0;
        resetControl();
        resetGrowthLeft();
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::reserve(uint64_t newSize) 
    {
        if (newSize > size + growthLeft) 
        {
            size_t m = capacityGrowthToLowerBound(newSize);
            resize(capacityNormalise(m));
        }
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::eraseMeta(const FlatHashMapIterator& iterator) 
    {
        --size;

        const uint64_t index = iterator.index;
        const uint64_t indexBefore = (index - GroupSse2Impl::WIDTH) & capacity;
        const auto emptyAfter = GroupSse2Impl(controlBytes + index).matchEmpty();
        const auto emptyBefore = GroupSse2Impl(controlBytes + indexBefore).matchEmpty();

        //We count how many consecutive non empty things we have to the right and to the left of 'it'.
        //If the sum is >= WIDTH then there is at least one probe window that might have seen a full group.
        const uint64_t trailingZeros = emptyAfter.trailingZeros();
        const uint64_t leadingZeros = emptyBefore.leadingZeros();
        const uint64_t zeros = trailingZeros + leadingZeros;
        bool wasNeverFull = emptyBefore && emptyAfter;
        wasNeverFull = wasNeverFull && (zeros < GroupSse2Impl::WIDTH);
         
        setControl(index, wasNeverFull ? CONTROL_BITMASK_EMPTY : CONTROL_BITMASK_DELETED);
        growthLeft += wasNeverFull;
    }

    template<typename K, typename V>
    FindResult FlatHashMap<K, V>::findOrPrepareInsert(const K& key) 
    {
        uint64_t hash = hashCalculate(key);
        ProbeSequence sequence = probe(hash);

        while (true) 
        {
            const GroupSse2Impl group{ controlBytes + sequence.getOffset() };
            for (int i : group.match(hash2(hash))) 
            {
                const KeyValue& keyValue = *(slots + sequence.getOffset(i));
                if (keyValue.key == key) 
                {
                    return { sequence.getOffset(i), false };
                }

                if (group.matchEmpty()) 
                {
                    break;
                }

                sequence.next();
            }
            return { prepareInsert(hash), true };
        }
    }

    template<typename K, typename V>
    FindInfo FlatHashMap<K, V>::findFirstNonFull(uint64_t hash) 
    {
        ProbeSequence sequence = probe(hash);

        while (true) 
        {
            const GroupSse2Impl group{ controlBytes + sequence.getOffset() };
            auto mask = group.matchEmptyOrDeleted();

            if (mask) 
            {
                return { sequence.getOffset(mask.lowerBitSet()), sequence.getIndex() };
            }

            sequence.next();
        }

        return FindInfo();
    }

    template<typename K, typename V>
    uint64_t FlatHashMap<K, V>::prepareInsert(uint64_t hash) 
    {
        FindInfo findInfo = findFirstNonFull(hash);
        if (growthLeft == 0 && !controlIsDeleted(controlBytes[findInfo.offset]))
        {
            rehashAndGrowIfNecessary();
            findInfo = findFirstNonFull(hash);
        }
        ++size;

        growthLeft -= controlIsEmpty(controlBytes[findInfo.offset]) ? 1 : 0;
        setControl(findInfo.offset, hash2(hash));
        return findInfo.offset;
    }

    template<typename K, typename V>
    ProbeSequence FlatHashMap<K, V>::probe(uint64_t hash) 
    {
        return ProbeSequence(hash1(hash, controlBytes), capacity);
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::rehashAndGrowIfNecessary() 
    {
        if (capacity == 0) 
        {
            resize(1);
        }
        else if (size <= capacityToGrowth(capacity) / 2) 
        {
            //Squash delete without growing if there is enough capacity.
            dropDeletesWithoutResize();
        }
        else 
        {
            resize(capacity * 2 + 1);
        }
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::dropDeletesWithoutResize() 
    {
        //This is the algorithm
        //mark all deleted slots as empty
        //mark all full slots as deleted.
        //for each slot marked as deleted
        //hash = Hash(element)
        //target = findFistNonFull(hash)
        //if target is in the same group
        //  mark slot as full.
        //else if target is empty
        //  transfer element to target
        //  mark slot as empty
        //  mark target is full
        //else if target is deleted
        //  swap current element with target element
        //  mark target as full.
        //  repeat procedure for current with moved from element (target)

        alignas(KeyValue) unsigned char raw[sizeof(KeyValue)];
        size_t totalProbeLength = 0;
        KeyValue* slot = reinterpret_cast<KeyValue*>(&raw);
        for (size_t i = 0; i != capacity; ++i) 
        {
            if (controlIsDeleted(controlBytes[i]) == false)
            {
                continue;
            }

            const KeyValue* currentSlot = slots + i;
            size_t hash = hashCalculate(currentSlot->key);
            auto target = findFirstNonFull(hash);
            size_t newi = target.offset;
            totalProbeLength += target.probeLength;

            //Verify if the old and new i fall the same group wrt the hash.
            //If they do, we don't need to move the object as it falls already in the best probe we can.
            const auto probeIndex = [&](size_t pos) 
            {
                return ((pos - probe(hash).getOffset()) & capacity) / GroupSse2Impl::WIDTH;
            };

            //Element doesn't move.
            if ((probeIndex(newi) == probeIndex(i))) 
            {
                setControl(i, hash2(hash));
                continue;
            }
            if (controlIsEmpty(controlBytes[newi])) 
            {
                //Transfer element to the empty spot.
                //setControl poisons/unpoisons the slots so we have to call it at the right time.
                setControl(newi, hash2(hash));
                memoryCopy(slots + newi, slots + i, sizeof(KeyValue));
                setControl(i, CONTROL_BITMASK_EMPTY);
            }
            else 
            {
                setControl(newi, hash2(hash));
                //Until we are done rehashing, deleted marks previously full slots.
                //swap i and newi elements.
                memoryCopy(slot, slots + i, sizeof(KeyValue));
                memoryCopy(slots + i, slots + newi, sizeof(KeyValue));
                memoryCopy(slots + newi, slot, sizeof(KeyValue));
                --i;
            }
        }

        resetGrowthLeft();
    }

    template<typename K, typename V>
    uint64_t FlatHashMap<K, V>::calculateSize(uint64_t newCapacity) 
    {
        return (newCapacity + GroupSse2Impl::WIDTH + newCapacity * (sizeof(KeyValue)));
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::initalisedSlots() 
    {
        char* newMemory = (char*)air_alloca(calculateSize(capacity), allocator);

        controlBytes = reinterpret_cast<int8_t*>(newMemory);
        slots = reinterpret_cast<KeyValue*>(newMemory + capacity + GroupSse2Impl::WIDTH);

        resetControl();
        resetGrowthLeft();
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::resize(uint64_t newCapacity) 
    {
        int8_t* oldControlBytes = controlBytes;
        KeyValue* oldSlots = slots;
        const uint64_t oldCapacity = capacity;

        capacity = newCapacity;

        initalisedSlots();

        size_t totalProbeLength = 0;
        for (size_t i = 0; i != oldCapacity; ++i) 
        {
            if (controlIsFull(oldControlBytes[i]))
            {
                const KeyValue* oldValue = oldSlots + i;
                uint64_t hash = hashCalculate(oldValue->key);

                FindInfo findInfo = findFirstNonFull(hash);

                uint64_t newi = findInfo.offset;
                totalProbeLength += findInfo.probeLength;

                setControl(newi, hash2(hash));

                memoryCopy(slots + newi, oldSlots + i, sizeof(KeyValue));
            }
        }

        if (oldCapacity) 
        {
            air_free(oldControlBytes, allocator);
        }
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::iteratorSkipEmptyOrDeleted(FlatHashMapIterator& iterator) 
    {
        int8_t* control = controlBytes + iterator.index;
        while (controlIsEmptyOrDeleted(*control)) 
        {
            uint32_t shift = GroupSse2Impl{ control }.countLeadingEmptyOrDeleted();
            control += shift;
            iterator.index += shift;
        }
        if (*control == CONTROL_BITMASK_SENTINEL) 
        {
            iterator.index = ITERATOR_END;
        }
    }

    //Sets the control byte, and if i < Group::WIDTH - 1, set the cloned byte at the end too.
    template<typename K, typename V>
    void FlatHashMap<K, V>::setControl(uint64_t i, int8_t h) 
    {
        controlBytes[i] = h;
        constexpr size_t clonedBytes = GroupSse2Impl::WIDTH - 1;
        controlBytes[((i - clonedBytes) & capacity) + (clonedBytes & capacity)] = h;
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::resetControl() 
    {
        memset(controlBytes, CONTROL_BITMASK_EMPTY, capacity + GroupSse2Impl::WIDTH);
        controlBytes[capacity] = CONTROL_BITMASK_SENTINEL;
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::resetGrowthLeft() 
    {
        growthLeft = capacityToGrowth(capacity) - size;
    }

    GroupSse2Impl::GroupSse2Impl(const int8_t* pos) 
    {
        control = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pos));
    }

    //Returns a bitmask representing the positions of slots that match hash.
    BitMask<uint32_t, GroupSse2Impl::WIDTH> GroupSse2Impl::match(int8_t hash) const
    {
        auto match = _mm_set1_epi8(hash);
        return BitMask<uint32_t, WIDTH>(_mm_movemask_epi8(_mm_cmpeq_epi8(match, control)));
    }

    //Returns a bitmask representing the position of empty slots.
    BitMask<uint32_t, GroupSse2Impl::WIDTH> GroupSse2Impl::matchEmpty() const
    {
        if (SSSE3_SUPPORT)
        {
            return BitMask<uint32_t, WIDTH>(_mm_movemask_epi8(_mm_sign_epi8(control, control)));
        }
        else 
        {
            return match(static_cast<int8_t>(CONTROL_BITMASK_EMPTY));
        }
    }

    BitMask<uint32_t, GroupSse2Impl::WIDTH> GroupSse2Impl::matchEmptyOrDeleted() const
    {
        auto special = _mm_set1_epi8(CONTROL_BITMASK_SENTINEL);
        return BitMask<uint32_t, WIDTH>(_mm_movemask_epi8(_mm_cmpgt_epi8(special, control)));
    }

    uint32_t GroupSse2Impl::countLeadingEmptyOrDeleted() const 
    {
        auto special = _mm_set1_epi8(CONTROL_BITMASK_SENTINEL);
        return trailingZerosU32(static_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpgt_epi8(special, control)) + 1));
    }

    void GroupSse2Impl::convertSpecialToEmptyAndFullToDelete(int8_t* destination) const
    {
        auto msbs = _mm_set1_epi8(static_cast<char>(-128));
        auto x126 = _mm_set1_epi8(126);

        if (SSSE3_SUPPORT)
        {
            auto res = _mm_or_si128(_mm_shuffle_epi8(x126, control), msbs);

            _mm_storeu_si128(reinterpret_cast<__m128i*>(destination), res);
        }
        else
        {
            auto zero = _mm_setzero_si128();
            auto specialMask = _mm_cmpgt_epi8(zero, control);
            auto res = _mm_or_si128(msbs, _mm_andnot_si128(specialMask, x126));

            _mm_storeu_si128(reinterpret_cast<__m128i*>(destination), res);
        }
    }
}