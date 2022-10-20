#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct node {
    char letter;
    int frequency;
    struct node *next;
    struct node *left;
    struct node *right;
} node;

node *htable[256];
node *head;
node *root;
node *new_tree_nodes[256];

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
        printf("0x%02x: ", r->letter);
        for (j = 0; j < length; j++) {
            printf("%c", code[j]);
        }

        printf("\n");
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

void print_linked_list(node *head) {
    while (head != NULL) {
        printf("%c:%d -> ", head->letter, head->frequency);
        head = head->next;
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

void build_htable(FILE *file) {
    int i;
    for (i = 0; i < 256; i++) {
        node *new_node = (node *) malloc(sizeof(node));
        if(!new_node) {
            perror("malloc");
            exit(3);
        }
        new_node->frequency = 0;
        htable[i] = new_node;
    }
    while (!feof(file)) {
        unsigned int thisChar = fgetc(file);
        if (thisChar != -1) {
            if (htable[thisChar]->frequency == 0) {
                htable[thisChar]->letter = thisChar;
            }
            htable[thisChar]->frequency++;
        }
    }
}

void free_memory_htable() {
    int i;
    for(i = 0; i < 256; i++) {
        free(htable[i]);
    }
}

int main(int argc, char *argv[]) {
    FILE *file;

    if (argc != 2) {
        fprintf(stderr, "usage: htable [file]\n");
        exit(4);
    }

    file = fopen(argv[1], "read");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    build_htable(file);
    insertion_sort();
    construct_linked_list();
    construct_tree(head);
    generate_huffman();
    free_memory_htable();
    free_tree_memory();
    fclose(file);
    return 0;
}
