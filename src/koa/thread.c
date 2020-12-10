#include "thread.h"

void mutex_lock(mutex_t* mtx) {
    while(mtx->lock);
    mtx->lock = 1;
}

void mutex_unlock(mutex_t* mtx) {
    mtx->lock = 0;
}

int mutex_locked(mutex_t* mtx) {
    return mtx->lock;
}