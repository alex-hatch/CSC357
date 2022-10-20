#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <limits.h>

typedef struct node {
    char letter;
    unsigned int frequency;
    struct node *next;
    struct node *left;
    struct node *right;
} node;

#define SIZE 4096

char *convert_hex_to_binary(char buff[]) {
    int i;
    int j;
    char *bin_string;
    bin_string = malloc(32);
    /*
    char temp;
    for(i = 1; i <= 2; i++) {
        temp = buff[i];
        buff[i] = buff[5 - i];
        buff[5 - i] = temp;
    }
     */

    printf("%c: ", buff[0]);
    for(i = 1; i < 5; i++) {
        for(j = 0; j < CHAR_BIT; j++)
        {
            bin_string[j] = buff[i]>>j&1;
            /*
            printf("%d", (buff[i]>>j)&1);
             */
        }
    }
    printf("%d\n", atoi(bin_string));
    return bin_string;
}

void parse_file(int fd) {
    char buff[SIZE];
    int num_unique_characters;
    int i;
    int j;
    int size_read;
    unsigned int decimal;
    /*
    node *new_node;
     */

    read(fd, buff, 1);
    num_unique_characters = buff[0];
    printf("Number of unique characters: %d\n", num_unique_characters + 1);
    /*printf("header: ");*/
    /*
    for(j = 0; j < 8; j++) {
        printf("%d", (buff[0]>>j)&1);
    }

    printf("\n");
     */
    decimal = 0;
    for(i = 0; i < num_unique_characters + 1; i++) {
        read(fd, buff, 5);
        /*
        new_node = (node *) malloc(sizeof(node));
        new_node->letter = buff[0];

        decimal = convert_hex_to_decimal(buff);
        new_node->frequency = decimal;

        printf("%c: %d\n", new_node->letter, new_node->frequency);
        convert_hex_to_binary(buff);
        */
        printf("%c: ", buff[0]);
        for(j = 0; j < 32; j++) {
            decimal = decimal + (pow(2, j) * ((buff[1]>>j)&1));
        }
        printf("%d", decimal);
        decimal = 0;
        printf("\n");

    }

    printf("body: ");
    size_read = read(fd, buff, SIZE);
    for(i = 0; i < size_read; i++) {
        unsigned char this_byte  = buff[i];
        for(j = 7; j >= 0; j--) {
            printf("%d", ((this_byte>>j)&1));
        }
    }
    printf("\n");
}

int main (int argc, char *argv[]) {
    int fd;

    if(argc != 2) {
        printf("Usage: ./hdecode [file]\n");
        exit(1);
    }

    fd = open(argv[1], O_RDONLY);

    parse_file(fd);
    return(0);
}
