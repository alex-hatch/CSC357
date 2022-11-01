#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

int main(int argc, char**argv) {
    int option_index;
    char *arg = NULL;
    while((option_index = getopt(argc, argv, "ctxvfS:")) != -1) {
        switch(option_index) {
            case 'f':
                /* Specifies archive filename */
                printf("case f\n");
                break;
            case 'c':
                /* create an archive */
                printf("case c\n");
                break;
            case 't':
                /* Print the table of contents of an archive */
                printf("case t\n");
                break;
            case 'x':
                /* Extract the contents of an archive */
                printf("case x\n");
                break;
            case 'v':
                /* Increases verbosity */
                printf("case v\n");
                break;
            case 'S':
                /* Be strict about standards compliance */
                printf("case S\n");
                break;
            default:
                printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
                exit(1);
        }
    }
    return 0;
}
