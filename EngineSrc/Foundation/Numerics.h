#ifndef NUMERICS_HDR
#define NUMERICS_HDR

#include "Platform.h"

namespace Air 
{
#define AIR_MATH_OVERFLOW_CHECK

#undef max
#undef min

    template<typename T>
    T max(const T& a, const T& b)
    {
        return a > b ? a : b;
    }

    template<typename T>
    T min(const T& a, const T& b)
    {
        return a < b ? a : b;
    }

    template<typename T>
    T clamp(const T& v, const T& a, const T& b)
    {
        return v < a ? a : (v > b ? b : v);
    }

    template<typename To, typename From>
    To safeCast(From a)
    {
        To result = (To)a;
        From check = (From)result;

        AIR_ASSERT(check == result);

        return result;
    }


    uint32_t ceilU32(float value);
    uint32_t ceilU32(double value);
    uint16_t ceilU16(float value);
    uint16_t ceilU16(double value);

    int32_t ceilI32(float value);
    int32_t ceilI32(double value);
    int16_t ceilI16(float value);
    int16_t ceilI16(double value);

    uint32_t floorU32(float value);
    uint32_t floorU32(double value);
    uint16_t floorU16(float value);
    uint16_t floorU16(double value);

    int32_t floorI32(float value);
    int32_t floorI32(double value);
    int16_t floorI16(float value);
    int16_t floorI16(double value);

    uint32_t roundU32(float value);
    uint32_t roundU32(double value);
    uint16_t roundU16(float value);
    uint16_t roundU16(double value);

    int32_t roundI32(float value);
    int32_t roundI32(double value);
    int16_t roundI16(float value);
    int16_t roundI16(double value);

    float getRandomValue(float min, float max);

    const float A_PI = 3.1415926538f;
    const float A_PI_2 = 1.57079632679f;
}

#endif // !NUMERICS_HDR
