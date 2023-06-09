// This is a utility file for the OLED display
// 
// THis includes utilities for intializing SPI

#ifndef __OLED_UTILS_H__
#define __OLED_UTILS_H__
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"

#include "rom_map.h"
#include "spi.h"
#include "prcm.h"

// SPI configuration defines
#define SPI_IF_BIT_RATE  20000000
#define TR_BUFF_SIZE     100

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define	RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

extern void SPI_Init(void);

extern void drawString(const char* string , int x, int y, unsigned int color, unsigned int bg_color, int size);

extern void drawFilledCircle(int x0, int y0, int r, unsigned int color);

#endif // __OLED_UTILS_H__