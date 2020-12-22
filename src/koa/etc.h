#ifndef KOA_ETC_H
#define KOA_ETC_H

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#define __MIN(A, B) ((A) < (B) ? (A) : (B))
#define __MAX(A, B) ((A) > (B) ? (A) : (B))

typedef struct {
    uint8_t r, g, b, a;
} color_t;

int color_eq(color_t, color_t);
int color_eqp(const color_t*, const color_t*);

void mfree(int n, ...);
uint8_t* ltoh(uint8_t*, int);
uint8_t* btoh(uint8_t*, int);

#endif
