#include "u.h"
#include "libc.h"
#include "dat.h"

// UART base addresses
enum {
	UART0 = 0x101f1000,
	UART1 = 0x101f2000,
	UART2 = 0x101f3000,
	UART3 = 0x10009000,
};

// register offsets, 32 bit wide
enum {
	// DATA register for writing output characters
	DR = 0x0,
};

// physical UART device descriptions
Uart physuart[] = {
    {
        .r = (void *)UART0,
    },
    {
        .r = (void *)UART1,
    },
    {
        .r = (void *)UART2,
    },
    {
        .r = (void *)UART3,
    },
};

// initializes UART
void
uartinit(void)
{
	// use UART0 for the console output
	consuart = &physuart[0];
}

// output a character to UART
void
uartputc(Uart *u, int c)
{
	u->r[DR] = c;
}
