#include <stdio.h>
#include "bmp.h"

int main(int argc, char *argv[])
{
    int i;
    struct data *d;
    char *bitmap_path;

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

    destroy_bitmap(d);
    return 0;
}
