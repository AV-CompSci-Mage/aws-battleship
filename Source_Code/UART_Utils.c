

// Standard includes
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "pin.h"
#include "uart.h"
#include "rom.h"
#include "rom_map.h"

#include "uart_if.h"

void UARTA1_Init(void(*pfnHandler)(void)){
    // initialize the second uart
    MAP_UARTConfigSetExpClk(UARTA1_BASE,MAP_PRCMPeripheralClockGet(CONSOLE_PERIPH),
                   UART_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                   UART_CONFIG_PAR_NONE));

    unsigned long ulstatus;

    UARTFIFODisable(UARTA1_BASE);

    // enable interrupts on the second uart
    UARTIntRegister(UARTA1_BASE, pfnHandler);

    UARTFIFOLevelSet(UARTA1_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    ulstatus = MAP_UARTIntStatus(UARTA1_BASE, false);

    UARTIntClear(UARTA1_BASE, ulstatus);

    UARTIntEnable(UARTA1_BASE, UART_INT_RX);
}
