#include <wnos/types.h>

extern void spinlock_acquire(spinlock_t* lock);
extern void spinlock_release(spinlock_t* lock);