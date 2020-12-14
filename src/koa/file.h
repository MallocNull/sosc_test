#ifndef KOA_FILE_H
#define KOA_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "koa/etc.h"
#include "koa/thread.h"
#include "koa/time.h"

//#define __STDERR err_ptr()
#define __STDERR stdout

FILE* err_ptr(void);
void err_out(const char*);

char* file_read(const char*);

// BEGIN BMP LOADING FUNCTIONS

typedef struct {
    uint8_t r, g, b;
} pixel_t;

typedef struct {
    char file[4096];
    uint32_t width, height, size,
             header_size, body_size;
    uint16_t bitpp, bytepp;
} bmp_meta_t;

typedef struct {
    bmp_meta_t data;

    uint32_t width, height;
    pixel_t** pixels;
} bmp_t;

bmp_t* bmp_load(const char*);
bmp_t* bmp_load_chunk(const char*, int, int, int, int);
int bmp_reload_chunk(bmp_t*, int, int, int, int);
int bmp_load_metadata(const char*, bmp_meta_t*);
void bmp_unload(bmp_t*);

#endif
