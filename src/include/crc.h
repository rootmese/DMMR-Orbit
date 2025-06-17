#ifndef __CRC_H__
#define __CRC_H__

#include <stdio.h>

static uint32_t crc32_table[256];

static inline void generate_crc32_table() {
    uint32_t poly = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++)
            crc = (crc & 1) ? (crc >> 1) ^ poly : crc >> 1;
        crc32_table[i] = crc;
    }
}

static inline uint32_t crc32_buffer(const uint8_t* data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++)
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    return crc ^ 0xFFFFFFFF;
}

static uint32_t crc32_file(const uint8_t* path) {
    FILE* f = fopen((char*)path, "rb");
    if (!f) return 0;

    uint8_t buf[1024];
    size_t bytes;
    uint32_t crc = 0xFFFFFFFF;

    while ((bytes = fread(buf, 1, sizeof(buf), f)) > 0)
        for (size_t i = 0; i < bytes; i++)
            crc = (crc >> 8) ^ crc32_table[(crc ^ buf[i]) & 0xFF];

    fclose(f);
    return crc ^ 0xFFFFFFFF;
}
#endif