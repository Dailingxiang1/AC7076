#ifndef __PRINTF_H__
#define __PRINTF_H__

#include <stdarg.h>
#include "typedef.h"
//#define NOFLOAT

// #ifdef __DEBUG

void put_u4hex(u8 dat);
void put_u8hex(unsigned char dat);
void put_u16hex(unsigned short dat);
void put_u32hex(unsigned int dat);
void put_buf(const u8 *buf, u32 len);
void printf_buf(u8 *buf, u32 len);


#ifdef __DEBUG
int printf(const char *format, ...);
int puts(const char *out);

#else

// #define put_u4hex(x)
// #define put_u8hex(x)
// #define put_u16hex(x)
// #define put_u32hex(x)
// #define put_buf(a, b)
// #define printf_buf(a, b)
#define printf(...)
#define puts(x)

#endif

int lib_putchar(int c);
extern int putchar(int a);


#endif

