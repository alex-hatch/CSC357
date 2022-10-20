typedef struct word_entry{
    char *word;
    int frequency;
    struct word_entry *next;
}word_entry;

word_entry **hash_table;

word_entry **top_k_words;

int hash(const char *word);

void rehash();

void clear_table();

void resize_table(int old_size);

int insert(word_entry *word);

void print_table();

void init_table();

void init_top_k(int k);

void print_top_k();

void clear_top_k();

void find_top_k();
