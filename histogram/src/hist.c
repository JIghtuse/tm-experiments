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

void *hist_updater(void *data);
void print_histogram();

int main(int argc, char *argv[])
{
    int i;
    long nproc;
    struct data *d;
    char *bitmap_path;
    double t;
    pthread_t *threads;

    nproc = -1;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s filename.bmp [number_of_threads]\n", argv[0]);
        return -1;
    }
    bitmap_path = argv[1];
    if (argc > 2) {
        nproc = atoi(argv[2]);
    }

    d = load_bitmap(bitmap_path);
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
    destroy_bitmap(d);
        
    print_histogram();
    printf("Time consumed: %f\n", t);

    return 0;
}

void *hist_updater(void *data)
{
    size_t i;
    struct data *d = data;
    for (i = 0; i < d->sz; ++i) {
        struct pixel p = d->pixels[i];

        unsigned int luminance = rY * p.red + gY * p.green + bY * p.blue;

        __sync_fetch_and_add(&histogram[luminance / BORDER], 1);
    }
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
