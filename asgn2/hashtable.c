#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOAD_FACTOR 0.75

double current_load_factor = 0;
int item_count_in_table = 0;
int unique_item_count = 0;
int table_size = 100;
int top_k;

typedef struct word_entry {
    char *word;
    int frequency;
    struct word_entry *next;
} word_entry;

word_entry **hash_table;

word_entry **top_k_words;

/* computer the hash for a word */
int hash(const char *word) {
    int hash_value;
    int word_length;
    int i;

    word_length = strlen(word);
    hash_value = 0;

    for (i = 0; i < word_length; i++) {
        hash_value += word[i];
        hash_value = hash_value * word[i];
    }

    return abs(hash_value % table_size);
}

/* when the hash table is resized, recompute the hashes of all the elements */
void rehash() {
    int i;
    word_entry **hash_table_temp = malloc(table_size * sizeof(word_entry));
    if(!hash_table_temp) {
        perror("malloc:");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < table_size; i++) {
        if (hash_table[i]) {
            word_entry *word = hash_table[i];
            while (word) {
                word_entry *next = word->next;
                int idx = hash(word->word);
                if (hash_table_temp[idx] != NULL) {
                    word->next = hash_table_temp[idx];
                } else {
                    word->next = NULL;
                }
                hash_table_temp[idx] = word;
                word = next;
            }
        }
    }

    for (i = 0; i < table_size; i++) {
        hash_table[i] = hash_table_temp[i];
    }
}

/* free up the memory used by hash table and top k words */
void clear_table() {
    int i;
    for (i = 0; i < table_size; i++) {
        if (hash_table[i]) {
            word_entry *word = hash_table[i];
            word_entry *next = word->next;
            while (word->next) {
                free(word->word);
                free(word);
                word = next;
                next = next->next;
            }
            free(word->word);
            free(word);
            hash_table[i] = NULL;
        }
    }
    item_count_in_table = 0;
    free(hash_table);
    free(top_k_words);
}

/* double the size of the table and reallocate the memory */
void resize_table(int old_size) {
    int new_size = 2 * old_size;
    hash_table = realloc(hash_table, new_size * sizeof(word_entry));
    if(!hash_table) {
        perror("realloc:");
        exit(EXIT_FAILURE);
    }
    table_size = new_size;
    current_load_factor = (double) item_count_in_table / table_size;
    rehash();
}

/* computer top k using a modified insertion sort algorithm */
void find_top_k() {
    int i;
    int j;
    for (i = 0; i < table_size; i++) {
        if (hash_table[i] != NULL) {
            word_entry *word = hash_table[i];
            while (word) {
                word_entry *key = word;
                j = top_k - 1;
                while (j >= 0 && top_k_words[j]->frequency < key->frequency) {
                    top_k_words[j + 1] = top_k_words[j];
                    j = j - 1;

                }
                while (j >= 0 && top_k_words[j]->frequency == key->frequency
                       && strcmp(top_k_words[j]->word, key->word) < 0) {
                    top_k_words[j + 1] = top_k_words[j];
                    j = j - 1;
                }
                top_k_words[j + 1] = key;
                word = word->next;
            }
        }
    }
}


void print_table() {
    int i;
    word_entry *entry;
    for (i = 0; i < table_size; i++) {
        if (hash_table[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else {
            printf("\t%i", i);
            entry = hash_table[i];
            while (entry != NULL) {
                printf("\t%s:%d", entry->word, entry->frequency);
                entry = entry->next;
            }
            printf("\n");
        }
    }
    printf("\n%d items in the table\n", item_count_in_table);
}

/* insert an item into the hash table */
void insert(word_entry *word) {
    int index;
    word_entry *entry;

    if (word == NULL)
        return;

    if (strcmp(word->word, "") == 0)
        return;

    if ((item_count_in_table / table_size) > LOAD_FACTOR) {
        resize_table(table_size);
    }

    index = hash(word->word);
    entry = hash_table[index];

    /* collision resolved by chaining */
    while (entry != NULL) {
        if (strcmp(entry->word, word->word) == 0) {
            entry->frequency = entry->frequency + 1;
            item_count_in_table++;
            return;
        }
        entry = entry->next;
    }

    /* the hashed index is available (no collision) */
    word->frequency = 1;
    word->next = hash_table[index];
    hash_table[index] = word;
    item_count_in_table++;
    unique_item_count++;
    return;
}

void init_table() {
    hash_table = malloc(table_size * sizeof(word_entry));
    if(!hash_table) {
        perror("malloc:");
        exit(EXIT_FAILURE);
    }
}

void init_top_k(int k) {
    int i;
    top_k_words = malloc(k * sizeof(word_entry));
    if(!top_k_words) {
        perror("malloc:");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < k; i++) {
        word_entry *wordEntry;
        wordEntry = malloc(sizeof(wordEntry));
        if(!wordEntry) {
            perror("malloc:");
            exit(EXIT_FAILURE);
        }
        wordEntry->word = "";
        wordEntry->frequency = -1;
        top_k_words[i] = wordEntry;
    }
    top_k = k;
}

void print_top_k() {
    int i;
    printf("The top %d words (out of %d) are:\n", top_k, unique_item_count);
    for (i = 0; i < top_k; i++) {
        word_entry *entry = top_k_words[i];
        if (entry->frequency != -1) {
            printf("%9d %s\n", entry->frequency, entry->word);
        }
    }
}
