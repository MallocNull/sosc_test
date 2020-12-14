#ifndef KOA_ETC_H
#define KOA_ETC_H

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

void mfree(int n, ...);
uint8_t* ltoh(uint8_t*, int);
uint8_t* btoh(uint8_t*, int);

#endif
