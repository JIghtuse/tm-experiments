#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
    pthread_t *threads;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s filename.bmp\n", argv[0]);
        return -1;
    }
    bitmap_path = argv[1];

    d = load_bitmap(bitmap_path);
    if (d != NULL) {
        printf("Bitmap %s loaded\n", bitmap_path);
    }

    nproc = sysconf(_SC_NPROCESSORS_ONLN);
    if (nproc < 0) {
        perror("sysconf");
        return -1;
    }
    threads = malloc(sizeof *threads * nproc);
    if (threads == NULL) {
        perror("malloc");
        return -1;
    }
    for (i = 0; i < nproc; ++i) {
        struct data chunk;
        chunk.sz = d->sz / nproc;
        chunk.pixels = &d->pixels[i * chunk.sz];

        if (pthread_create(&threads[i], NULL, hist_updater, &chunk)) {
            perror("pthread_create");
            nproc = i;
            break;
        }
    }
    for (i = 0; i < nproc; ++i) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
    destroy_bitmap(d);
        
    print_histogram();

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
