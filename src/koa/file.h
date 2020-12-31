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

#define KOA_COMPONENTS 4
#define KOA_R    1
#define KOA_G    2
#define KOA_B    4
#define KOA_A    8
#define KOA_RGB  7
#define KOA_RGBA F

typedef struct {
    char file[4096];
    uint32_t width, height, size,
             header_size, body_size;
    uint16_t bitpp, bytepp;
} bmp_meta_t;

typedef struct {
    bmp_meta_t data;

    uint32_t width, height;
    color_t** pixels;
} bmp_t;

bmp_t* bmp_load(const char*);

bmp_t* bmp_load_chunk(const char*, int, int, int, int);
int bmp_reload_chunk(bmp_t*, int, int, int, int);
int bmp_load_metadata(const char*, bmp_meta_t*);

uint8_t* bmp_gl_data(bmp_t*, int);
void bmp_gl_data_out(bmp_t*, uint8_t*, int);
uint32_t bmp_gl_data_size(bmp_t*, int);

void bmp_discard_pixels(bmp_t*);
void bmp_unload(bmp_t*);

#endif
