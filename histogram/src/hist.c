#include <pthread.h>
#include <stdio.h>
#include "bmp.h"

enum {
    MAX_PIXEL_VALUE = 255,
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
    struct data *d;
    char *bitmap_path;
    pthread_t t;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s filename.bmp\n", argv[0]);
        return -1;
    }
    bitmap_path = argv[1];

    d = load_bitmap(bitmap_path);
    if (d != NULL) {
        printf("Bitmap %s loaded\n", bitmap_path);
    }

    /* TODO: fill image histogram concurrently */
    pthread_create(&t, NULL, hist_updater, d);
    pthread_join(t, NULL);

    print_histogram();

    destroy_bitmap(d);
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
    printf("\n%s\n", "-----");
    for (i = 0; i < NBUCKETS; ++i) {
        printf("%d ", histogram[i]);
    }
    printf("\n%s\n", "-----");
}
