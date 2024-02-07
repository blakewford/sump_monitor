#include <cstdint>
#include <cstdio>

typedef FILE* BMP;

namespace bmp
{
    BMP create(uint32_t width, uint32_t height);
    void append_pixel(uint32_t* pixel, BMP bitmap);
    void close(BMP bitmap);
}