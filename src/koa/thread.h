#ifndef KOA_THREAD_H
#define KOA_THREAD_H

#define KOA_MTX_INIT { 0 }

typedef struct {
    int lock;
} mutex_t;

void mutex_lock(mutex_t*);
void mutex_unlock(mutex_t*);
int mutex_locked(mutex_t*);

#endif
