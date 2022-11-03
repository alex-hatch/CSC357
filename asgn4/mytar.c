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

#define MALLOC_SIZE 500

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

int extract_archive(char *tar_file) {
    int fd;
    int new_fd;
    char *name;
    char *mode;
    char *uid;
    char *gid;
    char *size;
    char *mtime;
    char *chksum;
    char *typeflag;
    char *linkname;
    char *magic;
    char *version;
    char *uname;
    char *gname;
    char *devmajor;
    char *devminor;
    char *prefix;
    char *contents;

    fd = open(tar_file, O_RDONLY);

    if (fd == -1) {
        perror(tar_file);
        exit(25);
    }

    if (!(name = (char *) malloc(NAME_SIZE))) {
        perror("malloc:");
        exit(9);
    }

    if (!(mode = malloc(MODE_SIZE))) {
        perror("malloc:");
        exit(10);
    }

    if (!(uid = malloc(UID_SIZE))) {
        perror("malloc:");
        exit(11);
    }

    if (!(gid = malloc(GID_SIZE))) {
        perror("malloc:");
        exit(12);
    }

    if (!(size = malloc(SIZE_SIZE))) {
        perror("malloc:");
        exit(13);
    }

    if (!(mtime = malloc(MTIME_SIZE))) {
        perror("malloc:");
        exit(14);
    }

    if (!(chksum = malloc(CHKSUM_SIZE))) {
        perror("malloc:");
        exit(15);
    }

    if (!(typeflag = malloc(TYPEFLAG_SIZE))) {
        perror("malloc:");
        exit(16);
    }

    if (!(linkname = malloc(LINKNAME_SIZE))) {
        perror("malloc:");
        exit(17);
    }

    if (!(magic = malloc(MAGIC_SIZE))) {
        perror("malloc:");
        exit(18);
    }

    if (!(version = malloc(VERSION_SIZE))) {
        perror("malloc:");
        exit(19);
    }

    if (!(uname = malloc(UNAME_SIZE))) {
        perror("malloc:");
        exit(20);
    }

    if (!(gname = malloc(GNAME_SIZE))) {
        perror("malloc:");
        exit(21);
    }

    if (!(devmajor = malloc(DEVMAJOR_SIZE))) {
        perror("malloc:");
        exit(22);
    }

    if (!(devminor = malloc(DEVMINOR_SIZE))) {
        perror("malloc:");
        exit(23);
    }

    if (!(prefix = malloc(PREFIX_SIZE))) {
        perror("malloc:");
        exit(24);
    }

    if (!(contents = malloc(atoi(size)))) {
        perror("malloc:");
        exit(25);
    }

    if (read(fd, name, NAME_SIZE) == -1) {
        perror(name);
        exit(26);
    }

    read(fd, mode, MODE_SIZE);
    read(fd, uid, UID_SIZE);
    read(fd, gid, GID_SIZE);
    read(fd, size, SIZE_SIZE);
    read(fd, mtime, MTIME_SIZE);
    read(fd, chksum, CHKSUM_SIZE);
    read(fd, typeflag, TYPEFLAG_SIZE);
    read(fd, linkname, LINKNAME_SIZE);
    read(fd, magic, MAGIC_SIZE);
    read(fd, version, VERSION_SIZE);
    read(fd, uname, UNAME_SIZE);
    read(fd, gname, GNAME_SIZE);
    read(fd, devmajor, DEVMAJOR_SIZE);
    read(fd, devminor, DEVMINOR_SIZE);
    read(fd, prefix, PREFIX_SIZE);

    lseek(fd, 12, SEEK_CUR);
    read(fd, contents, atoi(size));

    printf("name: %s\n", name);
    printf("mode: %s\n", mode);
    printf("uid: %s\n", uid);
    printf("gid: %s\n", gid);
    printf("size: %s\n", size);
    printf("mtime: %s\n", mtime);
    printf("chksum: %s\n", chksum);
    printf("typeflag: %s\n", typeflag);
    printf("linkname: %s\n", linkname);
    printf("magic: %s\n", magic);
    printf("version: %s\n", version);
    printf("uname: %s\n", uname);
    printf("gname: %s\n", gname);
    printf("devmajor: %s\n", devmajor);
    printf("devminor: %s\n", devminor);
    printf("prefix: %s\n", prefix);
    printf("contents: %s\n", contents);

    new_fd = open(name, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    write(new_fd, contents, atoi(size));

    free(name);
    free(mode);
    free(uid);
    free(gid);
    free(size);
    free(mtime);
    free(chksum);
    free(typeflag);
    free(linkname);
    free(magic);
    free(version);
    free(uname);
    free(gname);
    free(devmajor);
    free(devminor);
    free(prefix);
    free(contents);
    return 1;
}

int print_archive() {
    return 1;
}

int create_archive() {
    return 1;
}

int main(int argc, char **argv) {
    char *tarfile;
    char **paths = NULL;
    int count;
    int new_size;
    int path_count;
    int i;

    if (argc == 1) {
        fprintf(stderr, "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
        exit(1);
    }

    /* create an archive */
    if (strstr(argv[1], "c") != NULL) {
        if (t_flag == 1 || x_flag == 1) {
            fprintf(stderr, "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
            exit(4);
        }
        c_flag = 1;
    }

    /* Print the table of contents of an archive */
    if (strstr(argv[1], "t") != NULL) {
        if (c_flag == 1 || x_flag == 1) {
            fprintf(stderr, "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
            exit(5);
        }
        t_flag = 1;
    }

    /* Extract the contents of an archive */
    if (strstr(argv[1], "x") != NULL) {
        if (c_flag == 1 || t_flag == 1) {
            fprintf(stderr, "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
            exit(6);
        }
        x_flag = 1;
    }

    /* Increases verbosity */
    if (strstr(argv[1], "v") != NULL) {
        v_flag = 1;
    }

    /* Be strict about standards compliance */
    if (strstr(argv[1], "S") != NULL) {
        S_flag = 1;
    }

    /* Specifies archive filename */
    if (strstr(argv[1], "f") != NULL) {
        f_flag = 1;
    }

    /* f option is required */
    if (!f_flag) {
        fprintf(stderr, "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
        exit(3);
    }

    if (argc < 3) {
        fprintf(stderr, "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
        exit(8);
    }

    tarfile = argv[2];

    printf("Tar file name: %s\n", tarfile);

    paths = malloc(MALLOC_SIZE);
    if (!paths) {
        perror("malloc");
        exit(7);
    }
    count = 0;
    new_size = MALLOC_SIZE * 2;
    path_count = 0;
    for (i = 3; i < argc; i++) {
        if (count == MALLOC_SIZE) {
            paths = realloc(paths, new_size);
            count = 0;
            new_size += MALLOC_SIZE;
        }
        paths[i - 3] = argv[i];
        count++;
        path_count++;
    }


    for (i = 0; i < path_count; i++) {
        printf("%s\n", paths[i]);
    }

    printf("c: %d, t: %d, x: %d, v: %d, S: %d, f: %d\n", c_flag, t_flag, x_flag, v_flag, S_flag, f_flag);

    if (x_flag == 1) {
        extract_archive(tarfile);
    }

    free(paths);
    return 0;
}
