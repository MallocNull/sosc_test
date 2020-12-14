#include "file.h"

FILE* _err_fp;
char  _err_fn[20];

char* _err_file_name() {
    struct tm now = localtime_s(NULL);

    sprintf(_err_fn, "ERR_%04i%02i%02i%02i%02i%02i",
        now.tm_year + 1900,
        now.tm_mon + 1,
        now.tm_mday,
        now.tm_hour,
        now.tm_min,
        now.tm_sec
    );

    return _err_fn;
}

FILE* err_ptr(void) {
    if(_err_fp == NULL) {
        _err_fp = fopen(_err_file_name(), "wb");
        setvbuf(_err_fp, NULL, _IONBF, 0);
    }

    return _err_fp;
}

void err_out(const char* err) {
    static FILE* _fperr   = NULL;
    static char  _fnerr[] = "ERR_YYYYMMDDHHMMSS";

    char now[KOA_TIME_STRLEN];
    ctime_s(NULL, now);

    fprintf(__STDERR, "[%s] %s\n", now, err);
}

char* file_read(const char* file) {
    FILE* fp;
    char* content;
    long size;

    fp = fopen(file, "rb");
    if(fp == NULL)
        return NULL;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    content = malloc(sizeof(char) * size + 1);
    if(content == NULL)
        return NULL;

    content[size] = '\0';
    fread(content, 1, size, fp);
    fclose(fp);

    return content;
}

int _bmp_load_metadata(FILE* fp, bmp_meta_t* out) {


    return 1;
}

int bmp_load_metadata(const char* file, bmp_meta_t* out) {
    FILE* fp = fopen(file, "rb");
    if(fp == NULL)
        return 0;

    uint8_t buffer[4] = { 0 };
    fread(buffer, 1, 2, fp);
    if(memcmp("BM", buffer, 2) != 0)
        return 0;

    fread(buffer, 1, 4, fp);
    out->size = *(uint32_t*)ltoh(buffer, 4);

    fseek(fp, 0, SEEK_END);
    if(ftell(fp) != out->size)
        return 0;

    fseek(fp, 0x0A, SEEK_SET);
    fread(buffer, 1, 4, fp);
    out->header_size = *(uint32_t*)ltoh(buffer, 4);
    out->body_size = out->size - out->header_size;

    fseek(fp, 0x12, SEEK_SET);
    fread(buffer, 1, 4, fp);
    out->width = *(uint32_t*)ltoh(buffer, 4);
    fread(buffer, 1, 4, fp);
    out->height = *(uint32_t*)ltoh(buffer, 4);

    fseek(fp, 2, SEEK_CUR);
    fread(buffer, 1, 2, fp);
    out->bitpp = *(uint16_t*)ltoh(buffer, 2);
    out->bytepp = out->bitpp / 8;

    if(out->bitpp != 24 && out->bitpp != 32)
        return 0;

    strcpy(out->file, file);
    return 1;
}

bmp_t* bmp_load(const char* file) {
    return bmp_load_chunk(file, 0, 0, -1, -1);
}

bmp_t* bmp_load_chunk
    (const char* file, int x, int y, int width, int height)
{
    if(x < 0 || y < 0)
        return NULL;

    bmp_meta_t data;
    if(!bmp_load_metadata(file, &data))
        return NULL;

    bmp_t* bmp = malloc(sizeof(bmp_t));
    bmp->pixels = NULL;
    bmp->data = data;
    if(!bmp_reload_chunk(bmp, x, y, width, height)) {
        free(bmp);
        return NULL;
    }

    return bmp;
}

int bmp_reload_chunk(bmp_t* bmp, int x, int y, int width, int height) {
    bmp_meta_t* data = &bmp->data;
    if(data == NULL)
        return 0;

    FILE* fp = fopen(data->file, "rb");
    if(fp == NULL)
        return 0;

    if(x > data->width || y > data->height) {
        fclose(fp);
        return NULL;
    }

    if(bmp->pixels != NULL) {
        for(int i = 0; i < bmp->height; ++i)
            free(bmp->pixels[i]);
        free(bmp->pixels);
    }

    width = (width <= 0 || (x + width > data->width))
            ? data->width - x
            : width;
    height = (height <= 0 || (y + height > data->height))
             ? data->height - y
             : height;

    bmp->width = width;
    bmp->height = height;
    bmp->pixels = malloc(sizeof(pixel_t*) * height);
    for(int i = 0; i < height; ++i)
        bmp->pixels[i] = malloc(sizeof(pixel_t) * width);

    uint8_t buffer[4];
    for(int ay = 0; ay < height; ++ay) {
        fseek(fp,
              data->header_size
              + (data->width * (data->height - (y + ay) - 1) * data->bytepp)
              + x * data->bytepp,
              SEEK_SET
        );

        for(int ax = 0; ax < width; ++ax) {
            fread(buffer, 1, data->bytepp, fp);

            pixel_t* px = &bmp->pixels[ay][ax];
            px->b = buffer[0];
            px->g = buffer[1];
            px->r = buffer[2];
        }
    }

    fclose(fp);
    return 1;
}

void bmp_unload(bmp_t* bmp) {
    free(bmp->pixels);
    free(bmp);
}