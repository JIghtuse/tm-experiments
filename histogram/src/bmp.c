#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"
#include "files.h"

static int read_long(FILE *fp, unsigned long *ret);
static int read_short(FILE *fp, unsigned short *ret);
static void fseek_warn(FILE *fp, long offset, int whence);

static int read_long(FILE *fp, unsigned long *ret)
{
    unsigned char b[4];
    if (4 != fread(&b, sizeof(unsigned char), 4, fp)) {
        fprintf(stderr, "fread failed: %s\n", strerror(errno));
        return 1;
    }
    *ret = (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
    return 0;
}

static int read_short(FILE *fp, unsigned short *ret)
{
    unsigned char b[2];
    if (2 != fread(&b, sizeof(unsigned char), 2, fp)) {
        fprintf(stderr, "fread failed: %s\n", strerror(errno));
        return 1;
    }
    *ret = (b[1] << 8) | b[0];
    return 0;
}

static void fseek_warn(FILE *fp, long offset, int whence)
{
    if (0 != fseek(fp, offset, whence)) {
        fprintf(stderr, "fseek failed: %s\n", strerror(errno));
    }
}

struct data *load_bitmap(char *fname)
{
    char type[2];
    unsigned long data_offset;
    unsigned long head_size;
    unsigned long imgsize;
    unsigned short bitcount;
    struct data *dat = NULL;
    FILE *fp;

    fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        return dat;
    }

    if (1 != fread(&type, sizeof(type), 1, fp)) {
        fprintf(stderr, "fread failed: %s\n", strerror(errno));
        goto finish;
    }
    if (strncmp(type, "BM", 2)) {
        fprintf(stderr, "unsupported bmp format: %c%c\n", type[0], type[1]);
        goto finish;
    }
    fseek_warn(fp, 8, SEEK_CUR);
    read_long(fp, &data_offset);
    read_long(fp, &head_size);

    switch (head_size) {
    case 40: case 108:
        fseek_warn(fp, 10, SEEK_CUR);
        read_short(fp, &bitcount);
        break;
    default:
        fprintf(stderr, "%s\n", "Unsupported head size!");
        goto finish;
        break;
    }
    if (bitcount != 24) {
        fprintf(stderr, "%s\n", "Unsupported bitcount");
        goto finish;
    }
    fseek_warn(fp, 4, SEEK_CUR);
    read_long(fp, &imgsize);

    if (imgsize == 0) {
        off_t filesize = get_filesize(fname);
        imgsize = filesize - data_offset;
    }

    dat = malloc(sizeof *dat);
    dat->sz = 0;
    dat->pixels = malloc(imgsize);

    if (NULL == dat->pixels) {
        fprintf(stderr, "malloc failed: %s\n", strerror(errno));
        goto finish;
    }
    fseek_warn(fp, data_offset, SEEK_SET);
    if (imgsize != fread(dat->pixels, 1, imgsize, fp)) {
        fprintf(stderr, "fread failed: %s\n", strerror(errno));
        goto finish;
    }
    dat->sz = imgsize / (bitcount / 8);

finish:
    fclose(fp);
    return dat;
}

void destroy_bitmap(struct data *d)
{
    free(d->pixels);
    free(d);
}

