#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#define SIZE 4096

/* a node on the linked list / tree */
typedef struct node {
    char letter;
    int frequency;
    struct node *next;
    struct node *left;
    struct node *right;
} node;

/* letter code used to store the path in the tree to the letter */
typedef struct letter_code {
    char letter;
    char *path;
} letter_code;

/* for holding the count of letters */
node *htable[256];

/* the head of the linked list */
node *head;

/* the root of the huffman tree */
node *root;

/* the name of the out file */
char *out_file;

/* the file descriptor for the out file */
int out_fd;

/* new nodes created from generating the huffman tree */
node *new_tree_nodes[256];

/* all the occurring letters and their paths stored in an array */
letter_code *codes[256];

/* the number of codes */
int num_codes = 0;

/* free the codes that were generated */
void free_codes() {
    int i;
    for (i = 0; i < num_codes; i++) {
        free(codes[i]->path);
        free(codes[i]);
    }
}

/* convert binary to hex for storing in output file */
unsigned char binary_to_hex(char *bin) {
    long int binary;
    long int hex;
    int i;
    int remainder;
    int count;
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
    count = strlen(bin);
    a = hex;

    /* for the last byte, shift the important bits */
    for( ; count < 8; count++) {
        a = a << 1;
    }
    return a;
}

void write_file(int infile) {
    int i;
    int j;
    int num;
    char buff[SIZE];
    char *write_buff;
    char this_char;
    unsigned char hex;
    char *concat_bin;
    int concat_count;
    int k;
    int l;
    int new_size;
    int count;

    if (out_fd != 1) {
        out_fd = open(out_file,
                      O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    }
    if (out_fd == -1) {
        perror("outfile");
        exit(2);
    }

    num = num_codes - 1;
    if(write(out_fd, &num, 1) == -1) {
        perror("write");
        exit(11);
    }

    /* write the header of the file */
    for (i = 0; i < 255 && htable[i]->frequency == 0; i++);
    for (; i < 256; i++) {
        if(write(out_fd, &(htable[i]->letter), 1) == -1) {
            perror("write");
            exit(13);
        }
        if(write(out_fd, &(htable[i]->frequency), 4) == -1) {
            perror("write");
            exit(14);
        }
    }

    if(lseek(infile, 0, SEEK_SET) == -1) {
        perror("lseek");
        exit(14);
    }
    concat_count = 0;
    concat_bin = malloc(8);
    l = 0;
    write_buff = malloc(SIZE);
    if(!write_buff) {
        perror("malloc");
        exit(13);
    }
    new_size = SIZE;
    count = 0;

    /* write the body of the file */
    while ((num = read(infile, buff, SIZE)) > 0) {
        for (i = 0; i < num; i++) {
            this_char = buff[i];
            for (j = 0; j < 255; j++) {
                if (codes[j] != NULL && this_char == codes[j]->letter) {
                    for (k = 0; k < strlen(codes[j]->path); concat_count++) {
                        if (concat_count == 8) {
                            concat_bin[concat_count] = '\0';
                            hex = binary_to_hex(concat_bin);
                            free(concat_bin);
                            concat_bin = malloc(8);
                            if(!concat_bin) {
                                perror("malloc");
                                exit(14);
                            }
                            if (count == SIZE) {
                                new_size = new_size + SIZE;
                                write_buff =
                                        realloc(write_buff,
                                                new_size);
                            }
                            if(!write_buff) {
                                perror("realloc");
                                exit(15);
                            }
                            write_buff[l++] = hex;
                            concat_count = 0;
                            count++;
                        }
                        concat_bin[concat_count] = codes[j]->path[k++];
                    }
                }
            }
        }
    }
    hex = binary_to_hex(concat_bin);
    concat_bin[concat_count] = '\0';
    free(concat_bin);
    write_buff[l++] = hex;
    if(write(out_fd, write_buff, l) == -1) {
        perror("write");
        exit(16);
    }
    free(write_buff);
}

/* free the memory that the huffman tree construction crated */
void free_tree_memory() {
    int i;
    i = 0;

    while (new_tree_nodes[i] != NULL) {
        free(new_tree_nodes[i]);
        i++;
    }
}

/* tree traversal for generating codes */
void traverse_tree(node *r, char *code, int length) {
    int j;
    letter_code *this_code;
    char *path;

    if (r == NULL)
        return;
    if (r->left != NULL) {
        code[length] = '0';
        traverse_tree(r->left, code, length + 1);
    }
    if (r->right != NULL) {
        code[length] = '1';
        traverse_tree(r->right, code, length + 1);
    }

    /* if a leaf is found, write the code */
    if (r->left == NULL && r->right == NULL) {
        code[length] = '\0';
        this_code = (letter_code *) malloc(sizeof(letter_code));
        if (!this_code) {
            perror("malloc");
            exit(7);
        }
        this_code->letter = r->letter;
        path = malloc(sizeof(16));
        if (!path) {
            perror("malloc");
            exit(8);
        }
        for (j = 0; j < length; j++) {
            path[j] = code[j];
        }
        this_code->path = path;
        codes[num_codes++] = this_code;
    }
}

/* setup function for huffman tree traversal */
void generate_huffman() {
    char *aux;
    aux = (char *) malloc(16);
    if (!aux) {
        perror("malloc");
        exit(1);
    }
    traverse_tree(root, aux, 0);
    free(aux);
}

/* inserting linked list nodes in order based on frequency */
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

/* build the huffman tree */
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
        if (!new_parent) {
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

/* build the linked list from the hash table */
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

/* used for sorting the hash table based on letter frequency */
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

/* collect the frequency of words in a hash table */
void build_htable(int fd) {
    char buff[SIZE];
    int i;
    int num;
    for (i = 0; i < 256; i++) {
        node *new_node = (node *) malloc(sizeof(node));
        if (!new_node) {
            perror("malloc");
            exit(3);
        }
        new_node->frequency = 0;
        htable[i] = new_node;
    }

    while ((num = read(fd, buff, SIZE)) > 0) {
        if (num < 0) {
            perror("read");
            exit(6);
        }
        for (i = 0; i < num; i++) {
            if (htable[(int) buff[i]]->frequency == 0) {
                htable[(int) buff[i]]->letter = buff[i];
            }
            htable[(int) buff[i]]->frequency++;
        }
    }
}

/* free the memory dynamically allocated from the hash table */
void free_memory_htable() {
    int i;
    for (i = 0; i < 256; i++) {
        free(htable[i]);
    }
}

int main(int argc, char *argv[]) {
    int fd;

    if (argc > 3 || argc < 2) {
        fprintf(stderr, "usage: ./hencode infile [ outfile ]\n");
        exit(4);
    }

    if (argc == 3) {
        out_file = argv[2];
    } else if (argc == 2) {
        out_fd = 1;
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
    write_file(fd);
    free_memory_htable();
    free_tree_memory();
    free_codes();
    close(fd);
    return 0;
}
