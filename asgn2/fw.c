#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include "hashtable.h"

#define BUFFER_SIZE 10
#define SPACE 32
#define NEWLINE 10

char *read_long_word(FILE *file) {
    int count;
    int i;
    int this_char;
    int word_size;

    /* allocate some memory for the word */
    char *word = (char *) malloc(BUFFER_SIZE);
    if (!word) {
        perror("malloc:");
        exit(EXIT_FAILURE);
    }

    count = 0;
    i = 0;
    word_size = 0;

    while (!feof(file)) {
        /* if we have run out of allocated memory, allocate more */
        if (count == BUFFER_SIZE - 1) {
            word = (char *) realloc(word, strlen(word) + (BUFFER_SIZE));
            if (!word) {
                perror("realloc:");
                exit(EXIT_FAILURE);
            }
            count = 0;
        }

        /* grab a character from the file */
        this_char = tolower(fgetc(file));

        /* If it is a letter, append. If not, we've hit the end of the word */
        if (isalpha(this_char)) {
            word[i++] = this_char;
            count++;
            word_size++;
        }
        else {
            word[i] = '\0';
            return word;
        }
    }

    return word;
}

void parse_file(FILE *file) {
    /* parse file word by word and add to the hash map */
    while (!feof(file)) {
        char *word = read_long_word(file);
        word_entry *word_p = (word_entry *) malloc(sizeof(word_entry));
        if(!word_p) {
            perror("malloc:");
            exit(EXIT_FAILURE);
        }
        word_p->word = word;
        insert(word_p);
    }
}

int main(int argc, char *argv[]) {
    int k;
    int opt;
    FILE *file;
    k = 10;

    /* assume stdin */
    if (argc == 1) {
        init_table();
        init_top_k(k);
        parse_file(stdin);
        find_top_k();
        print_top_k();
        clear_table();
        return 0;
    }

    /* check for arguments in command line */
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                if (isdigit(*optarg)) {
                    k = atoi(optarg);
                    break;
                } else {
                    fprintf(stderr,
                            "usage: fw [-n num] [ file1 [ file2 [...] ] ]\n");
                    exit(EXIT_FAILURE);
                }
            default:
                fprintf(stderr,
                        "usage: fw [-n num] [ file1 [ file2 [...] ] ]\n");
                exit(EXIT_FAILURE);
        }
    }

    /* initialize the hash table as well as the top k array */
    init_table();
    init_top_k(k);

    /* read file by file */
    while (argv[optind]) {
        file = fopen(argv[optind], "read");
        if (file == NULL) {
            fprintf(stderr, "Unable to open file %s\n", argv[optind]);
        } else {
            parse_file(file);
            fclose(file);
        }
        optind++;
    }

    /* computer top k words */
    find_top_k();

    /* print top k array */
    print_top_k();

    /* put away the toys */
    clear_table();

    return 0;
}
