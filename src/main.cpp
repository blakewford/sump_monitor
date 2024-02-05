#include <cstdint>
#include <cstdio>

#include <string>
#include <unistd.h>

#include "mqtt.h"

#define FULL_GAUGE_AREA 52400.0f

#define MQTT_BROKER_HOSTNAME "homeassistant.local"
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
        while(pixels--)
        {
            read = fread(&p, sizeof(uint32_t), 1, capture);
            if(read != 1) continue;

            abgr[0] = (p >> 24) & 0xFF;
            abgr[1] = (p >> 16) & 0xFF;
            abgr[2] = (p >>  8) & 0xFF;
            abgr[3] = p & 0xFF;

            if(abgr[0] != 0xFF) continue;
            if(in_range(abgr[1], 0xFB, 0xFC)) // 0xFC, 252 Ideal
            {
                if(in_range(abgr[2], 0x8C, 0xF0)) // Heavy gradient 140 - 240
                {
                    if(in_range(abgr[3], 0x14, 0x64)) // Heavy gradient 20 - 100
                    {
                        count++;
                    }
                }
            }
        }

        fclose(capture);

        const int32_t BUFFER_SIZE = 4;
        char buffer[BUFFER_SIZE];
        memset(buffer, '\0', BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "%.0f", ((count/FULL_GAUGE_AREA)*100));

        int sock = mqtt::connect(MQTT_BROKER_HOSTNAME, MQTT_USERNAME, MQTT_PASSWORD);
        if(sock == -1) continue;

        mqtt::publish(sock, buffer);
        mqtt::disconnect(sock);
        close(sock);
    }

    return 0;
}
