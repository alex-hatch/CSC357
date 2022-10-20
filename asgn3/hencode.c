#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#define SIZE 4096

typedef struct node {
    char letter;
    int frequency;
    struct node *next;
    struct node *left;
    struct node *right;
} node;

typedef struct letter_code {
    char letter;
    char *path;
} letter_code;

node *htable[256];
node *head;
node *root;
node *new_tree_nodes[256];
letter_code *codes[256];
int num_codes = 0;
/*
void write_file() {
    int out;
    int num;
    int i;

    out = open("outfile", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    num = num_codes - 1;
    lseek(out, 0, SEEK_SET);
    write(out, &num, 1);
    for(i = 0; i < 255 && htable[i]->frequency == 0; i++);

    for( ; htable[i] != NULL; i++) {
        write(out, &(htable[i]->letter), 1);
        write(out, &(htable[i]->frequency), 4);
    }
}
 */

void free_codes() {
    int i;
    for(i = 0; i < num_codes; i++) {
        free(codes[i]->path);
        free(codes[i]);
    }
}

void print_codes()  {
    int i;
    for(i = 0; i < num_codes; i++) {
        printf("0x%02x: %s\n", codes[i]->letter, codes[i]->path);
    }
}

unsigned char binary_to_hex(char *bin) {
    long int binary;
    long int hex;
    int i;
    int remainder;
    unsigned char a;

    hex = 0;
    i = 1;
    binary = atoi(bin);
    while (binary != 0) {
        remainder = binary % 10;
        hex = hex + remainder * i;
        i = i * 2;
        binary = binary / 10;
    }
    a = hex;
    /*printf("size: %lu\n", sizeof(a));*/
    return a;
}

void write_file(int infile) {
    int out;
    int i;
    int j;
    int num;
    char buff[SIZE];
    char write_buff[SIZE];
    char this_char;
    unsigned char hex;
    char *concat_bin;
    int concat_count;
    int k;
    int l;

    out = open("outfile", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    num = num_codes - 1;
    write(out, &num, 1);
    for(i = 0; i < 255 && htable[i]->frequency == 0; i++);

    for( ; i < 256; i++) {
        /*printf("Writing %hhx: %d\n", htable[i]->letter, htable[i]->frequency);*/
        write(out, &(htable[i]->letter), 1);
        write(out, &(htable[i]->frequency), 4);
    }

    if(out == -1) {
        perror("outfile");
        exit(2);
    }

    lseek(infile, 0, SEEK_SET);
    concat_count = 0;
    concat_bin = malloc(8);
    l = 0;
    while((num = read(infile, buff, SIZE))  > 0) {
        for(i = 0; i < num; i++) {
            this_char = buff[i];
            for(j = 0; j < 255; j++) {
                if(codes[j] != NULL && this_char == codes[j]->letter) {
                    for(k = 0; k < strlen(codes[j]->path); concat_count++) {
                        if(concat_count == 8) {
                            concat_bin[concat_count] = '\0';
                            /*printf("concat bin: %s\n", concat_bin);*/
                            hex = binary_to_hex(concat_bin);
                            free(concat_bin);
                            concat_bin = malloc(8);
                            /*printf("%hx\n", hex);*/
                            /*write(out, &hex, 1);*/
                            write_buff[l++] = hex;
                            concat_count = 0;
                        }
                        concat_bin[concat_count] = codes[j]->path[k++];
                    }
                }
            }
        }
    }
    hex = binary_to_hex(concat_bin);
    concat_bin[concat_count] = '\0';
    /*printf("concat bin: %s\n", concat_bin);*/
    free(concat_bin);
    /*printf("%hx\n", hex);*/
    write_buff[l++] = hex;
    /*write(out, &hex, 1);*/
    write(out, write_buff, l);
}

void free_tree_memory() {
    int i;
    i = 0;

    while(new_tree_nodes[i] != NULL) {
        free(new_tree_nodes[i]);
        i++;
    }
}

void traverse_tree(node *r, char *code, int length) {
    int j;
    letter_code *this_code;
    char *path;

    if(r == NULL)
        return;
    if(r->left != NULL) {
        code[length] = '0';
        traverse_tree(r->left, code, length + 1);
    }
    if(r->right != NULL) {
        code[length] = '1';
        traverse_tree(r->right, code, length + 1);
    }
    if (r->left == NULL && r->right == NULL) {
        code[length] = '\0';
        this_code = (letter_code *) malloc(sizeof(letter_code));
        if(!this_code) {
            perror("malloc");
            exit(7);
        }
        this_code->letter = r->letter;
        path = malloc(sizeof(16));
        if(!path) {
            perror("malloc");
            exit(8);
        }
        /*printf("0x%02x: ", r->letter);*/
        for (j = 0; j < length; j++) {
            path[j] = code[j];
            /*printf("%c", code[j]);*/
        }
        this_code->path = path;
        codes[num_codes++] = this_code;
        /*printf("\n");*/
    }
}

void generate_huffman() {
    char *aux;
    aux = (char*) malloc(16);
    if(!aux) {
        perror("malloc");
        exit(1);
    }
    traverse_tree(root, aux, 0);
    free(aux);
}

void print_tree(node *r) {
    if (r == NULL)
        return;
    print_tree(r->left);
    printf("%d %c", r->frequency, r->letter);
    print_tree(r->right);
}

void in_order_insert(node *n) {
    node *runner;
    if (head == NULL) {
        head = n;
        return;
    }
    if (n->frequency < head->frequency) {
        n->next = head;
        head = n;
        return;
    }

    runner = head;
    while (runner->next != NULL && n->frequency > runner->next->frequency) {
        runner = runner->next;
    }
    n->next = runner->next;
    runner->next = n;
}

void construct_tree(node *list_head) {
    int i;
    int node_sum;
    node *node_one;
    node *node_two;
    node *new_parent;
    node_one = list_head;
    i = 0;
    while (node_one->next != NULL) {
        node_two = node_one->next;
        head = node_two->next;
        node_sum = node_one->frequency + node_two->frequency;
        new_parent = (node *) malloc(sizeof(node));
        if(!new_parent) {
            perror("malloc");
            exit(2);
        }
        new_parent->frequency = node_sum;
        new_parent->left = node_one;
        new_parent->right = node_two;
        new_tree_nodes[i++] = new_parent;
        in_order_insert(new_parent);
        node_one->next = NULL;
        node_two->next = NULL;
        root = new_parent;
        node_one = head;
    }
}

void print_linked_list(node *n) {
    while (n != NULL) {
        printf("%c:%d -> ", n->letter, n->frequency);
        n = n->next;
    }
    printf("null\n");
}


void construct_linked_list() {
    int i;
    i = 0;
    while (i < 255 && htable[i]->frequency == 0) {
        i++;
    }
    head = htable[i];
    while (i < 255 && htable[i + 1] != NULL) {
        htable[i]->next = htable[i + 1];
        i++;
    }
}

void insertion_sort() {
    int i, j;
    node *key;
    for (i = 1; i < 256; i++) {
        key = htable[i];
        j = i - 1;
        while (j >= 0 && htable[j]->frequency > key->frequency) {
            htable[j + 1] = htable[j];
            j = j - 1;
        }
        htable[j + 1] = key;
    }
}

void build_htable(int fd) {
    char buff[SIZE];
    int i;
    int num;
    for (i = 0; i < 256; i++) {
        node *new_node = (node *) malloc(sizeof(node));
        if(!new_node) {
            perror("malloc");
            exit(3);
        }
        new_node->frequency = 0;
        htable[i] = new_node;
    }

    while((num = read(fd, buff, SIZE))  > 0) {
        for(i = 0; i < num; i++) {
            if (htable[(int)buff[i]]->frequency == 0) {
                htable[(int)buff[i]]->letter = buff[i];
            }
            htable[(int)buff[i]]->frequency++;
        }
    }
    if(num < 0) {
        perror("read");
        exit(6);
    }
}

void free_memory_htable() {
    int i;
    for(i = 0; i < 256; i++) {
        free(htable[i]);
    }
}

void copy_file() {
    int num;
    char buff[SIZE];
    int in, out;
    in = open("test", O_RDONLY);
    if(in == -1) {
        perror("test");
        exit(1);
    }

    out = open("outfile", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);

    if(out == -1) {
        printf("rip\n");
        perror("outfile");
        exit(2);
    }

    while((num = read(in,  buff, SIZE))  > 0) {
        if(write(out, buff, num) < 0) {
            perror("write");
            exit(3);
        }
    }

    if(num < 0) {
        perror("read");
        exit(4);
    }
    close(out);
    close(in);
}

int main(int argc, char *argv[]) {
    int fd;

    if (argc != 2) {
        fprintf(stderr, "usage: ./hencode [file]\n");
        exit(4);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open:");
        exit(5);
    }
    build_htable(fd);
    insertion_sort();
    construct_linked_list();
    construct_tree(head);
    generate_huffman();
    /*print_codes();*/
    /*write_header();*/
    write_file(fd);
    free_memory_htable();
    free_tree_memory();
    free_codes();
    close(fd);
    return 0;
}
