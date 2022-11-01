#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

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

int extract_archive() {
    return 1;
}

int print_archive() {
    return 1;
}

int create_archive() {
    return 1;
}

int main(int argc, char**argv) {
    int option_index;
    int f_flag, c_flag, t_flag, x_flag, v_flag, S_flag;
    f_flag = c_flag = t_flag = x_flag = v_flag = S_flag = 0;

    if(argc == 1) {
        printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
        exit(1);
    }
    while((option_index = getopt(argc, argv, "ctxvfS:")) != -1) {
        switch(option_index) {
            case 'f':
                /* Specifies archive filename */
                printf("case f\n");
                f_flag = 1;
                break;
            case 'c':
                /* create an archive */
                printf("case c\n");
                c_flag = 1;
                break;
            case 't':
                /* Print the table of contents of an archive */
                printf("case t\n");
                t_flag = 1;
                break;
            case 'x':
                /* Extract the contents of an archive */
                printf("case x\n");
                x_flag = 1;
                break;
            case 'v':
                /* Increases verbosity */
                printf("case v\n");
                v_flag = 1;
                break;
            case 'S':
                /* Be strict about standards compliance */
                printf("case S\n");
                S_flag = 1;
                break;
            default:
                printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
                exit(2);
        }
    }

    /* f option is required */
    if(!f_flag) {
        printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
        exit(3);
    }
    return 0;
}
