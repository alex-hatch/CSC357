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

/* extract files from the archive */
int extract_archive(char *tar_file, char **paths,
                    int supplied_path, int path_count) {
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
    int size_read;
    int file_chunk_size;
    int converted_mode;
    int i;
    int j;
    int match;
    int our_sum;

    /* open the tar file for reading */
    if ((fd = open(tar_file, O_RDONLY)) == -1) {
        perror(tar_file);
        exit(25);
    }

    /* save the file chunk size */
    file_chunk_size = 0;
    while ((size_read = read(fd, NULL, 512)) != 0) {
        /*  reset fd back to beginning of block */
        lseek(fd, -(size_read + 1), SEEK_CUR);

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

        if (read(fd, name, NAME_SIZE) == -1) {
            perror(name);
            exit(26);
        }

        our_sum = 0;

        /* split the block into appropriate fields with
         * increment our_sum to verify with chksum */
        for (i = 0; i < NAME_SIZE; i++) {
            our_sum += (unsigned char) name[i];
        }

        if (read(fd, mode, MODE_SIZE) == -1) {
            perror(mode);
            exit(130);
        }
        for (i = 0; i < MODE_SIZE; i++) {
            our_sum += (unsigned char) mode[i];
        }

        if (read(fd, uid, UID_SIZE) == -1) {
            perror(uid);
            exit(131);
        }
        for (i = 0; i < UID_SIZE; i++) {
            our_sum += (unsigned char) uid[i];
        }

        if (read(fd, gid, GID_SIZE) == -1) {
            perror(gid);
            exit(132);
        }
        for (i = 0; i < GID_SIZE; i++) {
            our_sum += (unsigned char) gid[i];
        }

        if (read(fd, size, SIZE_SIZE) == -1) {
            perror(size);
            exit(133);
        }
        for (i = 0; i < SIZE_SIZE; i++) {
            our_sum += (unsigned char) size[i];
        }

        if (read(fd, mtime, MTIME_SIZE) == -1) {
            perror(mtime);
            exit(134);
        }
        for (i = 0; i < MTIME_SIZE; i++) {
            our_sum += (unsigned char) mtime[i];
        }

        if (read(fd, chksum, CHKSUM_SIZE) == -1) {
            perror(mtime);
            exit(135);
        }
        for (i = 0; i < CHKSUM_SIZE; i++) {
            our_sum += 32;
        }

        if (read(fd, typeflag, TYPEFLAG_SIZE) == -1) {
            perror(typeflag);
            exit(136);
        }
        for (i = 0; i < TYPEFLAG_SIZE; i++) {
            our_sum += (unsigned char) typeflag[i];
        }

        if (read(fd, linkname, LINKNAME_SIZE) == -1) {
            perror(linkname);
            exit(137);
        }
        for (i = 0; i < LINKNAME_SIZE; i++) {
            our_sum += (unsigned char) linkname[i];
        }

        if (read(fd, magic, MAGIC_SIZE) == -1) {
            perror(magic);
            exit(138);
        }
        for (i = 0; i < MAGIC_SIZE; i++) {
            our_sum += (unsigned char) magic[i];
        }

        if (read(fd, version, VERSION_SIZE) == -1) {
            perror(version);
            exit(139);
        }
        for (i = 0; i < VERSION_SIZE; i++) {
            our_sum += (unsigned char) version[i];
        }

        if (read(fd, uname, UNAME_SIZE) == -1) {
            perror(uname);
            exit(140);
        }
        for (i = 0; i < UNAME_SIZE; i++) {
            our_sum += (unsigned char) uname[i];
        }

        if (read(fd, gname, GNAME_SIZE) == -1) {
            perror(gname);
            exit(141);
        }
        for (i = 0; i < GNAME_SIZE; i++) {
            our_sum += (unsigned char) gname[i];
        }

        if (read(fd, devmajor, DEVMAJOR_SIZE) == -1) {
            perror(devmajor);
            exit(142);
        }
        for (i = 0; i < DEVMAJOR_SIZE; i++) {
            our_sum += (unsigned char) devmajor[i];
        }

        if (read(fd, devminor, DEVMINOR_SIZE) == -1) {
            perror(devminor);
            exit(143);
        }
        for (i = 0; i < DEVMINOR_SIZE; i++) {
            our_sum += (unsigned char) devminor[i];
        }

        if (read(fd, prefix, PREFIX_SIZE) == -1) {
            perror(prefix);
            exit(144);
        }
        for (i = 0; i < PREFIX_SIZE; i++) {
            our_sum += (unsigned char) prefix[i];
        }

        /* chksun failed: abort */
        if ((our_sum) != strtol(chksum, NULL, 8)) {
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
            exit(150);
        }

        /* go to the next block */
        lseek(fd, 12, SEEK_CUR);

        if (!(contents = malloc((strtol(size, NULL, 8)) + 1))) {
            perror("malloc:");
            exit(25);
        }

        /* read the contents of the file */
        if (read(fd, contents, (strtol(size, NULL, 8) + 1)) == -1) {
            perror(contents);
            exit(145);
        }


        /* concat prefix and name */
        if (strlen(prefix) != 0) {
            prefix[strlen(prefix)] = '/';
        }
        strcat(prefix, name);

        if (S_flag) {
            /* if strict mode, ensure magic null terminated
             * and version is 00
             */
            if (memcmp(magic, "ustar\0", MAGIC_SIZE) != 0) {
                fprintf(stderr, "incorrect magic\n");
                exit(100);
            }
            if (memcmp(version, "00", VERSION_SIZE) != 0) {
                fprintf(stderr, "incorrect version\n");
                exit(101);
            }
        } else {
            /* if non-strict, check for ustar */
            if (memcmp(magic, "ustar", MAGIC_SIZE - 1) != 0) {
                fprintf(stderr, "incorrect magic\n");
                exit(102);
            }
        }

        /* check if a specific path was supplied on the command line */
        if (supplied_path) {
            match = 0;
            for (j = 0; j < path_count; j++) {
                /* check to see if the
                 * header file name is a prefix of our target */
                if (strncmp(prefix, paths[j], strlen(paths[j])) == 0
                    && (prefix[strlen(paths[j])] == '\0'
                        || prefix[strlen(paths[j])] == '/')) {
                    match = 1;
                    if (memcmp(typeflag, "0", 1) == 0
                        || memcmp(typeflag, "\0", 1) == 0) {
                        /* we have a regular file */
                        converted_mode = (int) strtol(mode, NULL, 8);
                        if (((S_IXUSR | S_IXGRP | S_IXOTH)
                             & converted_mode) != 0) {
                            /* offer execute perms to everybody */
                            new_fd = open(name, O_WRONLY | O_CREAT,
                                          S_IRWXU, S_IRWXG, S_IRWXO);
                        } else {
                            new_fd = open(name, O_WRONLY | O_CREAT,
                                          S_IRUSR | S_IWUSR
                                          | S_IRGRP | S_IWGRP
                                          | S_IROTH | S_IWOTH);
                        }
                        /* write the contents of the file
                         * to the newly created file */
                        write(new_fd, contents, (strtol(size, NULL, 8)));
                        free(contents);
                        close(new_fd);

                        /* calculate how far back to lseek */
                        file_chunk_size = (int)
                                (strtol(size, NULL, 8) / 512) + 1;
                        lseek(fd, -(strtol(size,
                                           NULL, 8) + 1), SEEK_CUR);

                        /* lseek to next header */
                        lseek(fd, (512 * file_chunk_size), SEEK_CUR);
                    } else if (memcmp(typeflag, "5", 1) == 0) {
                        /* we've found a directory */
                        mkdir(prefix,
                              S_IRUSR | S_IWUSR | S_IXUSR
                              | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                        lseek(fd, -1, SEEK_CUR);
                    } else if (memcmp(typeflag, "2", 1) == 0) {
                        /* symbolic link */
                        if (symlink(linkname, prefix) == -1) {
                            perror("symlink");
                            exit(40);
                        }
                        lseek(fd, -1, SEEK_CUR);
                    } else {
                        fprintf(stderr, "Unsupported file type supplied\n");
                    }
                    /* verbose */
                    if (v_flag) {
                        printf("%s\n", prefix);
                    }
                }
            }
            /* this chunk of the tape was not
             * targeted by the command line input
             */
            if (!match) {
                /* skip past this */
                if (memcmp(typeflag, "0", 1) == 0
                    || memcmp(typeflag, "\0", 1) == 0) {
                    /* we have a regular file */
                    file_chunk_size = (int) (strtol(size, NULL, 8) / 512) + 1;
                    lseek(fd, -(strtol(size, NULL, 8) + 1), SEEK_CUR);
                    lseek(fd, (512 * file_chunk_size), SEEK_CUR);
                } else if (memcmp(typeflag, "5", 1) == 0) {
                    lseek(fd, -1, SEEK_CUR);
                } else if (memcmp(typeflag, "2", 1) == 0) {
                    /* symbolic link */
                    lseek(fd, -1, SEEK_CUR);
                }
                free(contents);
            }
        } else {
            /* no targets were supplied on the command line
             * extract all files
             */
            if (memcmp(typeflag, "0", TYPEFLAG_SIZE) == 0
                || memcmp(typeflag, "\0", TYPEFLAG_SIZE) == 0) {
                /* we have a regular file */
                converted_mode = (int) strtol(mode, NULL, 8);
                if (((S_IXUSR | S_IXGRP | S_IXOTH) & converted_mode) != 0) {
                    /* offer execute permissions to everybody */
                    new_fd = open(prefix, O_WRONLY | O_CREAT,
                                  S_IRWXU, S_IRWXG, S_IRWXO);
                } else {
                    /* nobody had execute permissions */
                    new_fd = open(prefix,
                                  O_WRONLY | O_CREAT,
                                  S_IRUSR | S_IWUSR | S_IRGRP
                                  | S_IWGRP | S_IROTH | S_IWOTH);
                }
                write(new_fd, contents, (strtol(size, NULL, 8)));
                free(contents);
                close(new_fd);
                /* calculate how many 512 block chunks this file occupied */
                file_chunk_size = (int) (strtol(size, NULL, 8) / 512) + 1;

                /* find the next file header */
                lseek(fd, -(strtol(size, NULL, 8) + 1), SEEK_CUR);
                lseek(fd, (512 * file_chunk_size), SEEK_CUR);
            } else if (memcmp(typeflag, "5", TYPEFLAG_SIZE) == 0) {
                mkdir(prefix, S_IRUSR | S_IWUSR
                              | S_IXUSR |
                              S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                lseek(fd, -1, SEEK_CUR);
            } else if (memcmp(typeflag, "2", TYPEFLAG_SIZE) == 0) {
                /* symbolic link */
                if (symlink(linkname, prefix) == -1) {
                    perror("symlink");
                    exit(40);
                }
                lseek(fd, -1, SEEK_CUR);
            } else {
                fprintf(stderr, "Unsupported file type supplied\n");
            }
            /* verbose list files as extracted */
            if (v_flag) {
                printf("%s\n", prefix);
            }
        }
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
    }
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
    int supplied_path;
    char **paths_copy;
    char *path_substring;
    char *new_path_piece;

    if (argc == 1) {
        fprintf(stderr, "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
        exit(1);
    }

    /* create an archive */
    if (strstr(argv[1], "c") != NULL) {
        if (t_flag == 1 || x_flag == 1) {
            fprintf(stderr,
                    "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
            exit(4);
        }
        c_flag = 1;
    }

    /* Print the table of contents of an archive */
    if (strstr(argv[1], "t") != NULL) {
        if (c_flag == 1 || x_flag == 1) {
            fprintf(stderr,
                    "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
            exit(5);
        }
        t_flag = 1;
    }

    /* Extract the contents of an archive */
    if (strstr(argv[1], "x") != NULL) {
        if (c_flag == 1 || t_flag == 1) {
            fprintf(stderr,
                    "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
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
        fprintf(stderr,
                "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
        exit(3);
    }

    if (argc < 3) {
        fprintf(stderr,
                "Usage: mytar [ctx][v][S]f tarfile [ path [ ... ] ]\n");
        exit(8);
    }

    tarfile = argv[2];

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

    paths_copy = malloc(path_count);
    for(i = 0; i < path_count; i++) {
        paths_copy[i] = strdup(paths[i]);
    }

    if (path_count > 0) {
        supplied_path = 1;
        for(i = 0; i < path_count; i++) {
            path_substring = strtok(paths_copy[i], "/");
            if(strcmp(path_substring, paths_copy[i]) == 0)
                continue;
            printf("HERE: %s\n", path_substring);
            mkdir(path_substring, S_IRUSR | S_IWUSR
                                  | S_IXUSR | S_IRGRP
                                  | S_IWGRP | S_IROTH | S_IWOTH);
            while((new_path_piece = strtok(NULL, "/")) != NULL) {
                strcat(path_substring, "/\0");
                strcat(path_substring, new_path_piece);
            }
        }
    }


    /*
    for (i = 0; i < path_count; i++) {
        printf("%s\n", paths_copy[i]);
    }
     */

    if (x_flag == 1) {
        extract_archive(tarfile, paths_copy, supplied_path, path_count);
    }

    free(paths);
    return 0;
}
