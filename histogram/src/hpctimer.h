/*
 * hpctimer.h: High-Resolution timers library.
 *
 * Copyright (C) 2011 Mikhail Kurnosov <mkurnosov@gmail.com>
 */

#ifndef HPCTIMER_H
#define HPCTIMER_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    HPCTIMER_SUCCESS = 0,
    HPCTIMER_FAILURE = 1
};

int hpctimer_initialize();
double hpctimer_wtime();

#ifdef __cplusplus
}
#endif

#endif /* HPCTIMER_H */

