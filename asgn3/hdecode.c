#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <limits.h>

#define SIZE 4096

typedef struct node {
    char letter;
    unsigned int frequency;
    struct node *next;
    struct node *left;
    struct node *right;
} node;

int *body;
int body_length;
node *head;
node *new_tree_nodes[256];
node *root;

void print_list() {
    node *runner = head;
    while(runner) {
        printf("%c: %d\n", runner->letter, runner->frequency);
        runner = runner->next;
    }
}

void traverse_tree() {
    int i;
    node *current_node = root;
    for(i = 0; i < body_length; i++) {
        if(current_node->left == NULL && current_node->right == NULL) {
            printf("%c", current_node->letter);
            current_node = root;
        }
        if(body[i] == 0) {
            current_node = current_node->left;
        }
        else if(body[i] == 1) {
            current_node = current_node->right;
        }
    }
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

void print_body() {
    int i;
    printf("body: ");
    for(i = 0; i < body_length; i++) {
        printf("%d", body[i]);
    }
    printf("\n");
}

void parse_file(int fd) {
    char buff[SIZE];
    int num_unique_characters;
    int i;
    int j;
    int k;
    int pow_count;
    int size_read;
    node *runner;
    unsigned int decimal;

    read(fd, buff, 1);
    num_unique_characters = buff[0];
    printf("Number of unique characters: %d\n", num_unique_characters + 1);

    decimal = 0;
    runner = head;
    for(i = 0; i < num_unique_characters + 1; i++) {
        node *new_node = malloc(sizeof(node));
        read(fd, buff, 5);
        /*printf("%c: ", buff[0]);*/
        new_node->letter = buff[0];
        pow_count = 0;
        for(j = 1; j < 5; j++) {
            for(k = 0; k < 8; k++) {
                decimal += (((buff[j]>>k)&1) * pow(2, pow_count));
                pow_count++;
            }
        }
        new_node->frequency = decimal;
        if(i == 0) {
            head = new_node;
        } else {
            runner->next = new_node;
        }
        runner = new_node;
        /*printf("%d\n", decimal);*/
        decimal = 0;
    }

    body = malloc(400);
    body_length = 0;
    size_read = read(fd, buff, SIZE);
    for(i = 0; i < size_read; i++) {
        unsigned char this_byte  = buff[i];
        for(j = 7; j >= 0; j--) {
            body[body_length] = (this_byte>>j)&1;
            body_length++;
        }
    }
}

int main (int argc, char *argv[]) {
    int fd;

    if(argc != 2) {
        printf("Usage: ./hdecode [file]\n");
        exit(1);
    }

    fd = open(argv[1], O_RDONLY);

    parse_file(fd);
    /*print_body();*/
    /*print_list();*/
    construct_tree(head);
    traverse_tree();
    return(0);
}
