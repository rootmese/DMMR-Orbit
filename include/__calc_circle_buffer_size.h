#ifndef ____CALC_CICLE_BUFFER_SIZE_H__
#define ____CALC_CICLE_BUFFER_SIZE_H__

static inline unsigned __calc_circle_buffer_size(void){}
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < 100000; i++) {
        __vcpy(dest, src, sizeof(struct node)); // tua cópia
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed = (t1.tv_sec - t0.tv_sec) * 1e9 + (t1.tv_nsec - t0.tv_nsec);
    return (double)elapsed / 100000;
}

#endif
