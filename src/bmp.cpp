#include "bmp.h"
#include <cstring>

//#define TRACE

namespace bmp
{
    const char* HEADER             = "BM";
    const uint32_t OFFSET          = 54;
    const uint32_t HEADER_SIZE     = 40;
    const uint16_t COLOR_PLANES    =  1;
    const uint16_t BITS_PER_PIXEL  = 24;
    const uint16_t BYTES_PER_PIXEL = BITS_PER_PIXEL/8;
    const uint32_t BLANK           = 0x00;

    BMP create(uint32_t width, uint32_t height)
    {
        FILE* f = nullptr;
#ifdef TRACE
        const uint32_t SIZE = OFFSET + (width*height)*(BYTES_PER_PIXEL);

        f = fopen("trace.bmp", "w");
        fwrite(HEADER, 1, strlen(HEADER), f);
        fwrite(&SIZE, sizeof(uint32_t), 1, f);
        fwrite(&BLANK, sizeof(uint32_t), 1, f);
        fwrite(&OFFSET, sizeof(uint32_t), 1, f);
        fwrite(&HEADER_SIZE, sizeof(uint32_t), 1, f);
        fwrite(&width, sizeof(uint32_t), 1, f);
        fwrite(&height, sizeof(uint32_t), 1, f);
        fwrite(&COLOR_PLANES, sizeof(uint16_t),1, f);
        fwrite(&BITS_PER_PIXEL, sizeof(uint16_t), 1, f);
        fwrite(&BLANK, sizeof(uint32_t), 1, f); // compression method
        fwrite(&BLANK, sizeof(uint32_t), 1, f);
        fwrite(&BLANK, sizeof(uint32_t), 1, f);
        fwrite(&BLANK, sizeof(uint32_t), 1, f);
        fwrite(&BLANK, sizeof(uint32_t), 1, f);
        fwrite(&BLANK, sizeof(uint32_t), 1, f);
#endif
        return f;
    }

    void append_pixel(uint32_t* pixel, BMP bitmap)
    {
#ifdef TRACE
        fwrite(pixel, BYTES_PER_PIXEL, 1, bitmap);
#endif
    }

    void close(BMP bitmap)
    {
#ifdef TRACE
        fclose(bitmap);
#endif
    }
}