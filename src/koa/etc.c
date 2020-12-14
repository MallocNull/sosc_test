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

uint8_t* _ord(uint8_t* buf, int length, int le) {
    static int end_chk = -1;
    if(end_chk == -1) {
        uint16_t chk = 0xB00B;
        end_chk = ((uint8_t*)&chk)[0] == 0x0B;
    }

    if((end_chk && le) || (!end_chk && !le))
        return buf;

    uint8_t tmp[8] = { 0 };
    for(int i = 0; i < length; ++i)
        tmp[i] = buf[length - i - 1];
    memcpy(buf, tmp, length);

    return buf;
}

uint8_t* ltoh(uint8_t* buf, int length) {
    return _ord(buf, length, 1);
}

uint8_t* btoh(uint8_t* buf, int length) {
    return _ord(buf, length, 0);
}