/*
 * hpctimer.c: High-Resolution timers library.
 *
 * Copyright (C) 2011 Mikhail Kurnosov <mkurnosov@gmail.com>
 */

#include <sys/time.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <inttypes.h>

#include "hpctimer.h"

#define NELEMS(v) (sizeof(v) / sizeof((v)[0]))

/*
 * Compilers macro:
 * __GNUC__ - GCC
 * __SUNPRO_C - Solaris Studio
 * __INTEL_COMPILER - Intel C++ Compiler
 * __xlC__ || __IBMC__ - IBM C Compiler
 * __PATHSCALE__ - PathScale Compiler
 * __PGI - PGI Compiler
 * __DECC - DEC Compiler
 * __HP_cc - HP Compiler
 * __SX - NEC SX Compiler
 * __COMO__ - Comeau C++
 * _CRAYC - Cray C Compiler
 * sgi || __sgi - SGI Compiler
 */

#if defined(__GNUC__)
#   define __inline__ __inline__
#   define __asm__ __asm__
#   define __volatile__ __volatile__
#elif defined(__SUNPRO_C)
#   define __inline__ __inline__
#   define __asm__ __asm__
#   define __volatile__ __volatile__
#endif

static int isinitialized = 0;
static uint64_t hpctimer_overhead;  /* Timer overhead (seconds) */
static uint64_t hpctimer_freq;      /* Timer frequency (ticks per usec) */

static double hpctimer_wtime_tsc();
static int hpctimer_tsc_initialize();
static __inline__ uint64_t hpctimer_gettsc();
static uint64_t hpctimer_measure_overhead();
static uint64_t hpctimer_calibrate_sleep(uint64_t overhead);

/* hpctimer_initialize: */
int hpctimer_initialize()
{
    isinitialized = 1;
    return hpctimer_tsc_initialize();
}

/*
 * hpctimer_sanity_check: Returns 1 if the results of measures
 *                        by timer are correct.
 */
int hpctimer_sanity_check()
{
    enum { NTESTS = 4 };
    double start, stop, currtime, prevtime = 0.0, err = 0.05;
    int sanity = 1, delay;
    
    for (delay = 1; delay < NTESTS; delay++) {
        start = hpctimer_wtime();
        sleep(delay);
        stop = hpctimer_wtime();
        currtime = stop - start;
        if (delay > 1) {
            if (fabs(prevtime - currtime / delay) > prevtime * err) {
                sanity = 0;
            }
            /*
            printf("# timer sleep %d sec.; timer result: %.6f; diff: %.6f\n",
                   delay - 1, currtime / delay, fabs(prevtime - currtime / delay));
            */
        }
        prevtime = currtime / delay;
    }
    return sanity;
}

/* hpctimer_wtime: Returns walltime in seconds. */
double hpctimer_wtime()
{
    if (!isinitialized)
        hpctimer_initialize();
    return hpctimer_wtime_tsc();
}

/* 
 * hpctimer_wtime_tsc: Returns TSC-based walltime in seconds. 
 */
static double hpctimer_wtime_tsc()
{
    return (double)(hpctimer_gettsc() - hpctimer_overhead) / (double)hpctimer_freq;
}

/*
 * hpctimer_tsc_initialize: Initializes TSC-based timer.
 *
 * The code is based on recommendations from manual of Intel Corp.
 * "Using the RDTSC Instruction for Performance Monitoring".
 */
static int hpctimer_tsc_initialize()
{
    printf("Initializing timer...\n");
    hpctimer_overhead = hpctimer_measure_overhead();
    hpctimer_freq = hpctimer_calibrate_sleep(hpctimer_overhead);
    return HPCTIMER_SUCCESS;    
}

/* 
 * hpctimer_gettsc: Returns TSC value. 
 */
static __inline__ uint64_t hpctimer_gettsc()
{
#if defined(__x86_64__)
    uint32_t low, high;                     
    __asm__ __volatile__(                   
        "xorl %%eax, %%eax\n"               
        "cpuid\n"                           
        ::: "%rax", "%rbx", "%rcx", "%rdx"  
    );                                      
    __asm__ __volatile__(                   
        "rdtsc\n"                           
        : "=a" (low), "=d" (high)           
    );                                      
    return ((uint64_t)high << 32) | low;     

#elif defined(__i386__)
    uint64_t tsc;
    __asm__ __volatile__(                   
        "xorl %%eax, %%eax\n"               
        "cpuid\n"                           
        ::: "%eax", "%ebx", "%ecx", "%edx"  
    );                                      
    __asm__ __volatile__(                   
        "rdtsc\n"                           
        : "=A" (tsc)                        
    );                                      
    return tsc;
#else
#   error "Unsupported platform"
#endif
}

/* hpctimer_measure_overhead: Returns overhead of TSC reading (in tics). */
static uint64_t hpctimer_measure_overhead()
{
    enum {
        TSC_OVERHEAD_NTESTS = 10
    };
    int i;
    uint64_t count, overhead = (uint64_t)~0x01;

    /* Make warm-up passes and determine timer overhead */
    for (i = 0; i < TSC_OVERHEAD_NTESTS; i++) {
        count = hpctimer_gettsc();
        count = hpctimer_gettsc() - count;
        if (count < overhead) {
            overhead = count;
        }
    }
    return overhead;
}

/* 
 * hpctimer_calibrate_adaptive: Returns number of TSC tics per second.
 *                              Adaptive algorithm based on sleep.
 */
/*
static uint64_t hpctimer_calibrate_adaptive(uint64_t overhead)
{
    enum {
        TSC_CALIBRATE_NTESTS = 2
    };
    int i;
    uint64_t count, freq;
        
    freq = (uint64_t)(~0x01);
    for (i = 0; i < TSC_CALIBRATE_NTESTS; i++) {
        count = hpctimer_gettsc();
        sleep(1);
        count = hpctimer_gettsc() - count - overhead;
        if (count < 0)
            count = 0;
        if (count < freq) {
            freq = count;
            i = 0;
        }
    }
    return freq;
}
*/

/* 
 * hpctimer_calibrate_sleep: Returns number of TSC tics per second.
 */
static uint64_t hpctimer_calibrate_sleep(uint64_t overhead)
{
    uint64_t count;
    int delay = 3;

    count = hpctimer_gettsc();
    sleep(delay);
    count = hpctimer_gettsc() - count - overhead;
    return count / delay;
}

/* 
 * hpctimer_calibrate_loop: Returns number of TSC tics per second.
 */
/*
static uint64_t hpctimer_calibrate_loop(uint64_t overhead)
{
    enum {
        TSC_CALIBRATE_NTESTS = 2
    };
    uint64_t count, countmin = (uint64_t)~0x01;
    struct timeval tv1, tv2;
    int i, j;
    __volatile__ int dummy = 0;
    
    for (i = 0; i < TSC_CALIBRATE_NTESTS; i++) {
        gettimeofday(&tv1, NULL);       
        count = hpctimer_gettsc();
        for (j = 0; j < 10000000; j++) {
            dummy++;
        }
        count = hpctimer_gettsc() - count - overhead;
        gettimeofday(&tv2, NULL);
        if (count < 0)
            count = 0;
        if (count < countmin)
            countmin = count;
    }
    return countmin * 1000000 / (tv2.tv_sec * 1000000 + tv2.tv_usec - 
                                 tv1.tv_sec * 1000000 - tv1.tv_usec);
}
*/

