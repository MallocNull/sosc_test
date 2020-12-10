#ifndef KOA_TIME_H
#define KOA_TIME_H

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "thread.h"

#define KOA_TIME_STRLEN 26

struct tm localtime_s(time_t*);
struct tm gmtime_s(time_t*);

void asctime_s(struct tm*, char*);
char* asctime_sm(struct tm*);

void ctime_s(time_t*, char*);
char* ctime_sm(time_t*);

#endif
