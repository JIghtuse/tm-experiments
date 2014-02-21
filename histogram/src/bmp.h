#ifndef BMP_H_
#define BMP_H_

struct pixel {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
};

struct data {
    size_t sz;
    struct pixel *pixels;
};

struct data *load_bitmap(char *fname);
void destroy_bitmap(struct data *d);

#endif /* BMP_H_ */
