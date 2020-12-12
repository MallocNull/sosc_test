#include "etc.h"

void mfree(int n, ...) {
    va_list args;
    va_start(args, n);

    if(n > 0)
        for(int i = 0; i < n; ++i)
            free(va_arg(args, void*));
    else {
        void* ptr = NULL;
        for(;;) {
            ptr = va_arg(args, void*);
            if(ptr == NULL)
                break;

            free(ptr);
        }
    }

    va_end(args);
}