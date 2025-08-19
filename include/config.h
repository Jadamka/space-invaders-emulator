#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct config_t
{
    uint32_t windowWidth = 100;
    uint32_t windowHeight = 100;
    uint32_t windowScale = 8;

    uint32_t foregroundColor = 0xFFFFFFFF;
    uint32_t backgroundColor = 0x00000000;
} config_t;

#endif // CONFIG_H
