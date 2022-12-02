#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>
#include "mush.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/errno.h>

#define READ 0
#define WRITE 1

/* executes a command */
void execute_command(char **argv) {
    if ((execvp(argv[0], argv)) == -1) {
        perror(argv[0]);
        _exit(2);
    }
}

/* closes pipes */
void close_pipes(int *old, int *new) {
    if ((close(old[READ]) == -1)) {
        perror("close old read");
        exit(11);
    }
    if ((close(old[WRITE]) == -1)) {
        perror("close old write");
        exit(12);
    }
    if ((close(new[READ]) == -1)) {
        perror("close new read");
        exit(11);
    }
    if ((close(new[WRITE]) == -1)) {
        perror("close new write");
        exit(12);
    }
}

void shell(FILE *input, int file) {
    int pid;
    int i;
    char *user_input;
    struct pipeline *pl;
    int old[2];
    int new[2];
    int input_fd;
    int output_fd;

    while (1) {
        fflush(stdin);
        fflush(stdout);
        if (!file) {
            printf("8-P ");
        }
        /* get user input from the command line */
        user_input = readLongString(input);

        /* Logic for SIGINT */
        if (errno == EINTR) {
            /* SIGINT at this point */
            fflush(stdin);
            fflush(stderr);
            errno = 0;
            continue;
        }

        /* Logic for ^D */
        if (feof(input)) {
            yylex_destroy();
            exit(0);
        }

        /* If no input, just continue to next line prompt */
        if (strcmp(user_input, "") == 0) {
            continue;
        }

        /* Split the user_input into stages based on pipes. Crack! */
        pl = crack_pipeline(user_input);

        /* if crack_pipeline encounters an error */
        if (pl == NULL) {
            shell(stdin, 0);
        }

        /*
        if (pl->length == 0) {
            continue;
        }
         */
        if (strcmp(pl->stage->argv[0], "cd") == 0) {
            if (pl->length > 1) {
                fprintf(stderr, "usage: cd <dir>\n");
            } else {
                if ((chdir(pl->stage->argv[1])) == -1) {
                    perror(pl->stage->argv[1]);
                    shell(stdin, 0);
                }
            }
            continue;
        }
        if ((pipe(old)) == -1) {
            perror("pipe");
            exit(-1);
        }
        if ((pipe(new)) == -1) {
            perror("pipe");
            exit(-1);
        }
        i = 0;
        while (i < pl->length) {
            pid = fork();
            if (pid < 0) {
                perror("pid");
                exit(-1);
            }
            if (pid == 0) {
                if (pl->stage->inname != NULL) {
                    if ((input_fd = open(pl->stage->inname,
                                         O_RDONLY, 0)) == -1) {
                        perror("open");
                        exit(-1);
                    }
                    if ((dup2(input_fd, STDIN_FILENO)) == -1) {
                        perror("dup2");
                        exit(-1);
                    }
                }
                if (pl->stage->outname != NULL) {
                    if ((output_fd = open(pl->stage->outname, O_CREAT | O_RDWR,
                                          S_IRUSR | S_IWUSR | S_IRGRP
                                          | S_IWGRP | S_IROTH
                                          | S_IWOTH)) == -1) {
                        perror("open");
                        exit(-1);
                    }
                    if ((dup2(output_fd, STDOUT_FILENO)) == -1) {
                        perror("dup2");
                        exit(-1);
                    }
                }
                /* no piping needed, one command */
                if (i == 0 && pl->length == 1) {
                    /*printf("RUNNING IN CHILD 0: %s\n", pl->stage->argv[0]);*/
                    close_pipes(old, new);
                    yylex_destroy();
                    execute_command(pl->stage->argv);
                }
                    /* first pipe */
                else if (i == 0) {
                    /*printf("RUNNING IN CHILD FIRST PIPE: %s\n", pl->stage->argv[0]);*/
                    if ((dup2(old[WRITE], STDOUT_FILENO)) == -1) {
                        perror("dup2");
                        exit(-1);
                    }
                    close_pipes(old, new);
                    yylex_destroy();
                    execute_command(pl->stage->argv);
                }
                    /* last pipe but only two commands */
                else if (i == pl->length - 1 && pl->length == 2) {
                    if ((dup2(old[READ], STDIN_FILENO)) == -1) {
                        perror("dup2");
                        exit(-1);
                    }
                    close_pipes(old, new);
                    yylex_destroy();
                    execute_command(pl->stage->argv);
                }
                    /* last pipe */
                else if (i == pl->length - 1) {
                    /*printf("RUNNING IN CHILD LAST PIPE: %s\n", pl->stage->argv[0]);*/
                    if ((dup2(new[READ], STDIN_FILENO)) == -1) {
                        perror("dup2");
                        exit(-1);
                    }
                    close_pipes(old, new);
                    yylex_destroy();
                    execute_command(pl->stage->argv);
                }
                    /* middle pipes */
                else {
                    /*printf("RUNNING IN CHILD MIDDLE PIPE: %s\n", pl->stage->argv[0]);*/
                    if ((dup2(old[READ], STDIN_FILENO)) == -1) {
                        perror("dup2");
                        exit(-1);
                    }
                    if ((dup2(new[WRITE], STDOUT_FILENO)) == -1) {
                        perror("dup2");
                        exit(-1);
                    }
                    close_pipes(old, new);
                    yylex_destroy();
                    execute_command(pl->stage->argv);
                }
            }
            i++;
            pl->stage++;
            fflush(stdin);
        }
        free(user_input);
        close_pipes(old, new);
        for (; i > 0; i--) {
            pl->stage--;
            wait(NULL);
        }
        free_pipeline(pl);
    }
}

void sigquit_handle() {
    printf("\n");
}

int main(int argc, char *argv[]) {
    FILE *input;
    struct sigaction sa;

    sa.sa_handler = sigquit_handle;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    if (argc == 1) {
        shell(stdin, 0);
    } else if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            perror("fopen");
            exit(-1);
        }
        shell(input, 1);
    }
    yylex_destroy();
    return 0;
}
