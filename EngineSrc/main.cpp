#include "Foundation/Memory.h"
#include "Foundation/Array.h"
#include "Foundation/Platform.h"

#include <iostream>

using namespace Air;

int main()
{
    Array<uint32_t> numberArray;
    HeapAllocator heapAllocator;
    heapAllocator.init(32 * 1024 * 1024);
    numberArray.init(&heapAllocator, 5);
    numberArray.push(1);
    numberArray.push(2);
    numberArray.push(3);
    numberArray.push(4);
    numberArray.push(5);

    for (uint32_t i = 0; i < numberArray.size; ++i)
    {
        std::cout << "Elements are : " << numberArray[i] << std::endl;
    }

    return 0;
}