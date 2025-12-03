#ifndef __UART_H__
#define __UART_H__

#include "typedef.h"

int putchar(int a);
void uart_init(const char *tx_io, u32 baud);
void uart_close(void);

#endif
