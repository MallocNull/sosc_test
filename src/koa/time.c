#include "time.h"

struct tm localtime_s(time_t* tm) {
    static mutex_t mtx = KOA_MTX_INIT;
    mutex_lock(&mtx);

    time_t now = time(NULL);
    struct tm copy;
    copy = *localtime(tm == NULL ? &now : tm);

    mutex_unlock(&mtx);
    return copy;
}

struct tm gmtime_s(time_t* tm) {
    static mutex_t mtx = KOA_MTX_INIT;
    mutex_lock(&mtx);

    time_t now = time(NULL);
    struct tm copy;
    copy = *gmtime(tm == NULL ? &now : tm);

    mutex_unlock(&mtx);
    return copy;
}

void asctime_s(struct tm* tm, char* out) {
    static mutex_t mtx = KOA_MTX_INIT;
    mutex_lock(&mtx);

    strcpy(out, asctime(tm));

    mutex_unlock(&mtx);
}

char* asctime_sm(struct tm* tm) {
    char* out = malloc(sizeof(char) * KOA_TIME_STRLEN);
    asctime_s(tm, out);

    return out;
}

void ctime_s(time_t* tm, char* out) {
    time_t now = time(NULL);
    struct tm tmp = localtime_s(tm == NULL ? &now : tm);
    asctime_s(&tmp, out);
}

char* ctime_sm(time_t* tm) {
    char* out = malloc(sizeof(char) * KOA_TIME_STRLEN);
    ctime_s(tm, out);

    return out;
}