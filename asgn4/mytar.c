#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "mytar.h"

#define NAME_OFFSET 0
#define MODE_OFFSET 100
#define UID_OFFSET 108
#define GID_OFFSET 116
#define SIZE_OFFSET 124
#define MTIME_OFFSET 136
#define CHKSUM_OFFSET 148
#define TYPEFLAG_OFFSET 156
#define LINKNAME_OFFSET 157
#define MAGIC_OFFSET 257
#define VERSION_OFFSET 263
#define UNAME_OFFSET 265
#define GNAME_OFFSET 297
#define DEVMAJOR_OFFSET 329
#define DEVMINOR_OFFSET 337
#define PREFIX_OFFSET 345

#define NAME_SIZE 100
#define MODE_SIZE 8
#define UID_SIZE 8
#define GID_SIZE 8
#define SIZE_SIZE 12
#define MTIME_SIZE 12
#define CHKSUM_SIZE 8
#define TYPEFLAG_SIZE 1
#define LINKNAME_SIZE 100
#define MAGIC_SIZE 6
#define VERSION_SIZE 2
#define UNAME_SIZE 32
#define GNAME_SIZE 32
#define DEVMAJOR_SIZE 8
#define DEVMINOR_SIZE 8
#define PREFIX_SIZE 155

int f_flag, c_flag, t_flag, x_flag, v_flag, S_flag;

uint32_t extract_special_int(const char *where, int len) {
    /* For interoperability with GNU tar. GNU seems to
    * set the high–order bit of the first byte, then
    * treat the rest of the field as a binary integer
    * in network byte order.
    * I don’t know for sure if it’s a 32 or 64–bit int, but for
    * this version, we’ll only support 32. (well, 31)
    * returns the integer on success, –1 on failure.
    * In spite of the name of htonl(), it converts int32 t
    */
    int32_t val = -1;
    if ((len >= sizeof(val)) && (where[0] & 0x80)) {
        /* the top bit is set, and we have space
        * extract the last four bytes */
        val = *(int32_t *) (where + len - sizeof(val));
        val = ntohl(val); /* convert to host byte order */
    }
    return val;
}

int insert_special_int(char *where, size_t size, int32_t val) {
    /* For interoperability with GNU tar. GNU seems to
    * set the high–order bit of the first byte, then
    * treat the rest of the field as a binary integer
    * in network byte order.
    * Insert the given integer into the given field
    * using this technique. Returns 0 on success, nonzero
    * otherwise
    */
    int err = 0;
    if (val < 0 || (size < sizeof(val))) {
        /* if it’s negative, bit 31 is set and we can’t use the flag
        * if len is too small, we can’t write it. Either way, we’re
        * done.
        */
        err++;
    } else {
        /* game on....*/
        memset(where, 0, size); /* Clear out the buffer */
        *(int32_t *) (where + size - sizeof(val)) = htonl(val); /* place the int */
        *where |= 0x80; /* set that high–order bit */
    }
    return err;
}

int extract_archive() {
    return 1;
}

int print_archive() {
    return 1;
}

int create_archive() {
    return 1;
}

int main(int argc, char **argv) {
    int option_index;

    if (argc == 1) {
        printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
        exit(1);
    }
    while ((option_index = getopt(argc, argv, "ctxvfS:")) != -1) {
        switch (option_index) {
            case 'f':
                /* Specifies archive filename */
                f_flag = 1;
                break;
            case 'c':
                /* create an archive */
                c_flag = 1;
                break;
            case 't':
                /* Print the table of contents of an archive */
                t_flag = 1;
                break;
            case 'x':
                /* Extract the contents of an archive */
                x_flag = 1;
                break;
            case 'v':
                /* Increases verbosity */
                v_flag = 1;
                break;
            case 'S':
                /* Be strict about standards compliance */
                S_flag = 1;
                break;
            default:
                printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
                exit(2);
        }
    }

    /* f option is required */
    if (!f_flag) {
        printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
        exit(3);
    }
    return 0;
}
