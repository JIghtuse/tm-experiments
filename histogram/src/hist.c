#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hpctimer.h"
#include "bmp.h"

enum {
    MAX_PIXEL_VALUE = 256,
    NBUCKETS = 16,
    BORDER = MAX_PIXEL_VALUE / NBUCKETS
};

/* luminance values */
const double rY = 0.2126;
const double gY = 0.7152;
const double bY = 0.0722;

int histogram[NBUCKETS];

#ifdef _USE_MUTEX
static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
#endif

void *hist_updater(void *data);
void print_histogram();
void print_usage(char *binary);

int main(int argc, char *argv[])
{
    int i;
    int nproc;
    struct data *d;
    char *bitmap_path;
    double t;
    pthread_t *threads;

    nproc = -1;
    if (argc < 2) {
        bitmap_path = NULL;
    } else {
        bitmap_path = argv[1];
    }
    if (argc > 2) {
        nproc = atoi(argv[2]);
    }

    /* 
     * TODO: handle args better, for example allow set nproc without setting
     * path
     */

    if (bitmap_path == NULL) {
        d = generate_fake_bitmap(FAKE_BMAP_SIZE);
    } else {
        d = load_bitmap(bitmap_path);

    }
    if (d != NULL) {
        printf("Bitmap %s loaded\n", bitmap_path);
    }

    if (nproc <= 0) {
        nproc = sysconf(_SC_NPROCESSORS_ONLN);
        if (nproc < 0) {
            perror("sysconf");
            return -1;
        }
    }
    threads = malloc(sizeof *threads * nproc);
    if (threads == NULL) {
        perror("malloc");
        return -1;
    }
    printf("%d threads to be launched\n", nproc);

    t = hpctimer_wtime();
    for (i = 0; i < nproc; ++i) {
        int offset;
        struct data *chunk = malloc(sizeof *chunk);

        /*
         * XXX: some pixels would not be processed if number of pixels not
         * divided by threads evenly
         */
        chunk->sz = d->sz / nproc;
        if (chunk->sz * (i + 1) > d->sz) {
            chunk->sz = d->sz - chunk->sz * i;
        }
        offset = chunk->sz * i;
        chunk->pixels = &(d->pixels[offset]);

        if (pthread_create(&threads[i], NULL, hist_updater, chunk)) {
            perror("pthread_create");
            nproc = i;
            break;
        }
    }
    for (i = 0; i < nproc; ++i) {
        pthread_join(threads[i], NULL);
    }
    t = hpctimer_wtime() - t;
    free(threads);
    printf("%d threads finished work\n", nproc);
    destroy_bitmap(d);
        
    print_histogram();
    printf("Time consumed: %f\n", t);

    return 0;
}

void *hist_updater(void *data)
{
    size_t i;
    struct data *d = data;
#ifdef _USE_TSX
    __transaction_atomic {
#endif
        for (i = 0; i < d->sz; ++i) {
            struct pixel p = d->pixels[i];

            unsigned int luminance = rY * p.red + gY * p.green + bY * p.blue;

#if defined _USE_TSX
            ++histogram[luminance/BORDER];
#elif defined _USE_ATOMIC
            __sync_fetch_and_add(&histogram[luminance / BORDER], 1);
#elif defined _USE_MUTEX
            pthread_mutex_lock(&mut);
            ++histogram[luminance/BORDER];
            pthread_mutex_unlock(&mut);
#endif
        }
#ifdef _USE_TSX
    }
#endif
    free(d);
    return NULL;
}

void print_histogram()
{
    int i;
    printf("\nStandard Objective Pixel Luminance Histogram\n");
    printf("%s\n", "-----");
    printf("%s", "L: ");
    for (i = 0; i < NBUCKETS; ++i) {
        int lhs = BORDER * i;
        int rhs = lhs + BORDER;
        printf("%3d-%3d ", lhs, rhs);
    }
    printf("\n%s\n", "-----");
    printf("%s", "H: ");
    for (i = 0; i < NBUCKETS; ++i) {
        printf("%7d ", histogram[i]);
    }
    printf("\n%s\n", "-----");
}

void print_usage(char *binary)
{
    fprintf(stderr, "Usage: %s [filename.bmp] [number_of_threads]\n", binary);
}
