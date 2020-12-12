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
