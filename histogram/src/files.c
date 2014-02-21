#include "files.h"

off_t get_filesize(char *fname)
{
    struct stat sb;

    if (stat(fname, &sb)) {
        return -1;
    }
    return sb.st_size;
}

