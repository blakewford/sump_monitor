#include <cstdint>
#include <cstdio>

#include <string>
#include <unistd.h>

#include "bmp.h"
#include "mqtt.h"

#define FULL_GAUGE_AREA 54000.0f
#define HIGH_LEVEL_ALARM_PERCENTAGE 90.0f
#define HIGH_LEVEL_DRAIN_CUTOFF 30 // seconds
#define MINIMUM_REFILL_TIME    120 // seconds

#define BLUE 0x0000FF // RGB

#define MQTT_BROKER_HOSTNAME "homeassistant"
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

bool in_range(uint8_t value, uint8_t low, uint8_t high)
{
    return (value >= low) && (value <= high);
}

int main()
{

#ifdef __ANDROID__
    std::string command = "screencap > ";
    const char* CAPTURE_FILE = "/data/local/tmp/capture";
#else
    std::string command = "ls ";
    const char* CAPTURE_FILE = "capture";
#endif
    command += CAPTURE_FILE;

    time_t mark = 0;
    time_t last_drain = time(nullptr);
    while(true)
    {
        sleep(5);

        system(command.c_str());

        FILE* capture = fopen(CAPTURE_FILE, "rb");

        uint32_t w, h, s, f;
        size_t read = fread(&w, sizeof(uint32_t), 1, capture);
        if(read != 1) continue;
        read = fread(&h, sizeof(uint32_t), 1, capture);
        if(read != 1) continue;
        read = fread(&s, sizeof(uint32_t), 1, capture);
        if(read != 1) continue;
        read = fread(&f, sizeof(uint32_t), 1, capture);
        if(read != 1) continue;

        uint32_t p;
        int32_t count = 0;
        size_t pixels = w*h;
        uint8_t abgr[sizeof(uint32_t)]; // ANDROID_BITMAP_FORMAT_RGBA_8888

        auto bitmap = bmp::create(w, h);
        while(pixels--)
        {
            read = fread(&p, sizeof(uint32_t), 1, capture);
            if(read != 1) continue;

            abgr[0] = (p >> 24) & 0xFF;
            abgr[1] = (p >> 16) & 0xFF;
            abgr[2] = (p >>  8) & 0xFF;
            abgr[3] = p & 0xFF;

            if(abgr[0] != 0xFF) continue;

            bool mark = false;
            uint32_t pixel  = (abgr[3] << 16) | (abgr[2] <<  8) | abgr[1];
            if(in_range(abgr[1], 0xFB, 0xFD)) // 0xFC, 252 Ideal
            {
                if(in_range(abgr[2], 0x8C, 0xBA)) // Heavy gradient 140 - 186
                {
                    if(in_range(abgr[3], 0x14, 0x72)) // Heavy gradient 20 - 114
                    {
                        mark = true;
                        count++;
                    }
                }
            }

            if(mark)
            {
                pixel = BLUE;
            }
            bmp::append_pixel(&pixel, bitmap);
        }
        bmp::close(bitmap);

        fclose(capture);

        float liquid_level_ratio = ((count/FULL_GAUGE_AREA)*100);

        const int32_t BUFFER_SIZE = 4;
        char buffer[BUFFER_SIZE];
        memset(buffer, '\0', BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "%.0f", liquid_level_ratio);

        int sock = mqtt::connect(MQTT_BROKER_HOSTNAME, MQTT_USERNAME, MQTT_PASSWORD);
        if(sock == -1) continue;

        mqtt::publish(sock, buffer);

        if(liquid_level_ratio > HIGH_LEVEL_ALARM_PERCENTAGE)
        {
            if(mark == 0)
            {
                mark = time(nullptr);
            }
            else
            {
                time_t now = time(nullptr);
                time_t diff = now - mark;
                if(diff > HIGH_LEVEL_DRAIN_CUTOFF)
                {
                    if((now - last_drain) > MINIMUM_REFILL_TIME) // We can't refill faster than this
                    {
                        mqtt::publish(sock, "RUN_GRINDER");
                        last_drain = time(nullptr);
                    }
                }
            }
        }
        else
        {
            mark = 0;
        }

        mqtt::disconnect(sock);
        close(sock);
    }

    return 0;
}
