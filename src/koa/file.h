#ifndef KOA_FILE_H
#define KOA_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "koa/thread.h"
#include "koa/time.h"

#define __STDERR err_ptr()

FILE* err_ptr(void);
void err_out(const char*);

char* file_read(const char*);

#endif
