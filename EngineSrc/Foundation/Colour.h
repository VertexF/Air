#ifndef COLOUR_HDR
#define COLOUR_HDR

#include "Platform.h"

namespace Air 
{
    struct Colour 
    {
        void set(float red, float green, float blue, float alpha);

        float r() const;
        float g() const;
        float b() const;
        float a() const;

        Colour operator=(uint32_t colour);

        static uint32_t fromU8(int8_t r, int8_t g, int8_t b, int8_t a);
        static uint32_t getDistinctColour(uint32_t index);

        static const uint32_t red =         0xFF0000FF;
        static const uint32_t green =       0xFF00FF00;
        static const uint32_t blue =        0xFFFF0000;
        static const uint32_t yellow =      0xFF00FFFF;
        static const uint32_t black =       0xFF000000;
        static const uint32_t white =       0xFFFFFFFF;
        static const uint32_t transparent = 0x00000000;

        uint32_t abgr;
    };
}//Air

#endif // !COLOUR_HDR
