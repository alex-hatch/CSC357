#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

int main(int argc, char**argv) {
    int option_index;
    int f_flag, c_flag, t_flag, x_flag, v_flag, S_flag;
    f_flag = c_flag = t_flag = x_flag = v_flag = S_flag = 0;

    if(argc == 1) {
        printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
        exit(1);
    }
    while((option_index = getopt(argc, argv, "ctxvfS:")) != -1) {
        switch(option_index) {
            case 'f':
                /* Specifies archive filename */
                printf("case f\n");
                f_flag = 1;
                break;
            case 'c':
                /* create an archive */
                printf("case c\n");
                c_flag = 1;
                break;
            case 't':
                /* Print the table of contents of an archive */
                printf("case t\n");
                t_flag = 1;
                break;
            case 'x':
                /* Extract the contents of an archive */
                printf("case x\n");
                x_flag = 1;
                break;
            case 'v':
                /* Increases verbosity */
                printf("case v\n");
                v_flag = 1;
                break;
            case 'S':
                /* Be strict about standards compliance */
                printf("case S\n");
                S_flag = 1;
                break;
            default:
                printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
                exit(2);
        }
    }

    if(!f_flag) {
        printf("Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
        exit(3);
    }
    return 0;
}
