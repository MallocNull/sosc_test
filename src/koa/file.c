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
        return 0;
    }

    if(bmp->pixels != NULL)
        bmp_discard_pixels(bmp);

    width = (width <= 0 || (x + width > data->width))
            ? data->width - x
            : width;
    height = (height <= 0 || (y + height > data->height))
             ? data->height - y
             : height;

    bmp->width = width;
    bmp->height = height;
    bmp->pixels = malloc(sizeof(color_t*) * height);
    for(int i = 0; i < height; ++i)
        bmp->pixels[i] = malloc(sizeof(color_t) * width);

    uint8_t buffer[4];
    for(int ay = 0; ay < height; ++ay) {
        fseek(
            fp,
            data->header_size
                + (data->width * (data->height - (y + ay) - 1) * data->bytepp)
                + x * data->bytepp,
            SEEK_SET
        );

        for(int ax = 0; ax < width; ++ax) {
            fread(buffer, 1, data->bytepp, fp);

            bmp->pixels[ay][ax][_B_] = buffer[0];
            bmp->pixels[ay][ax][_G_] = buffer[1];
            bmp->pixels[ay][ax][_R_] = buffer[2];
            bmp->pixels[ay][ax][_A_] = 0xFF;
        }
    }

    fclose(fp);
    return 1;
}

int _bmp_comp_cnt(int col) {
    int cnt = 0;
    for(int i = 0; i < KOA_COMPONENTS; ++i)
        cnt += (col >> i) & 0x1;

    return cnt;
}

uint8_t* bmp_gl_data(bmp_t* bmp, int col) {
    float* out = malloc(bmp_gl_data_size(bmp, col));
    bmp_gl_data_out(bmp, out, col);
    return out;
}

void bmp_gl_data_out(bmp_t* bmp, uint8_t* out, int col) {
    const int comps = _bmp_comp_cnt(col);
    for(uint32_t y = 0; y < bmp->height; ++y) {
        for(uint32_t x = 0; x < bmp->width; ++x) {
            int at = 0;
            for(int i = 0; i < KOA_COMPONENTS; ++i) {
                if(!((col >> i) & 0x1))
                    continue;

                out[y * bmp->width * comps + x * comps + at] =
                    bmp->pixels[y][x][i];
                ++at;
            }
        }
    }
}

uint32_t bmp_gl_data_size(bmp_t* bmp, int col) {
    return sizeof(uint8_t) * _bmp_comp_cnt(col)
           * bmp->width * bmp->height;
}

void bmp_discard_pixels(bmp_t* bmp) {
    if(bmp->pixels == NULL)
        return;

    for(int i = 0; i < bmp->height; ++i)
        free(bmp->pixels[i]);
    free(bmp->pixels);
    bmp->pixels = NULL;
}

void bmp_unload(bmp_t* bmp) {
    bmp_discard_pixels(bmp);
    free(bmp->pixels);
    free(bmp);
}