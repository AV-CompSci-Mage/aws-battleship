#include "OLED_Utils.h"

// SPI_Init
void SPI_Init(void) {
     // Enable the SPI module clock
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    // Reset the SPI peripheral
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    // Reset SPI
    MAP_SPIReset(GSPI_BASE);

    // Configure SPI interface
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVELOW |
                     SPI_WL_8));

    // Enable SPI for communication
    MAP_SPIEnable(GSPI_BASE);
}


void drawString(const char* string , int x, int y, unsigned int color, unsigned int bg_color, int size){
    int i = 0;
    while(string[i] != '\0'){
        //    void drawChar(int x, int y, unsigned char c, unsigned int color, unsigned int bg, unsigned char size);
        drawChar(x + i * 6 * size, y, string[i], color, bg_color, size);
        i++;
    }
}

void drawFilledCircle(int x0, int y0, int r, unsigned int color){
    int x = r;
    int y = 0;
    int err = 0;
 
    while (x >= y)
    {
        drawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
        drawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
        drawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
        drawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);
 
        y += 1;
        err += 1 + 2*y;
        if (2*(err-x) + 1 > 0)
        {
            x -= 1;
            err += 1 - 2*x;
        }
    }
}