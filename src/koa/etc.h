#ifndef KOA_ETC_H
#define KOA_ETC_H

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#define __MIN(A, B) ((A) < (B) ? (A) : (B))
#define __MAX(A, B) ((A) > (B) ? (A) : (B))

#define _R_ 0
#define _G_ 1
#define _B_ 2
#define _A_ 3

typedef uint8_t color_t[4];

int color_eq(const color_t, const color_t);
int color_eqp(const color_t, const color_t);

void mfree(int n, ...);
uint8_t* ltoh(uint8_t*, int);
uint8_t* btoh(uint8_t*, int);

#endif
