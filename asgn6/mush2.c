#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>
#include "mush.h"

#define READ 0
#define WRITE 1

void execute_command(char **argv) {
    if((execvp(argv[0], argv)) == -1) {
        perror(argv[0]);
        _exit(2);
    }
}

void close_pipes(int *old, int *new) {
    if((close(old[READ]) == -1)) {
        perror("close old read");
        exit(11);
    }
    if((close(old[WRITE]) == -1)) {
        perror("close old write");
        exit(12);
    }
    if((close(new[READ]) == -1)) {
        perror("close new read");
        exit(11);
    }
    if((close(new[WRITE]) == -1)) {
        perror("close new write");
        exit(12);
    }
}

void shell() {
    int pid;
    int i;
    char *user_input;
    struct pipeline *pl;
    int old[2];
    int new[2];

    while(1) {
        fflush(stdin);
        fflush(stdout);
        printf("8-P ");
        user_input = readLongString(stdin);
        if(feof(stdin)) {
            yylex_destroy();
            free(user_input);
            exit(0);
        }
        if(strcmp(user_input, "") == 0) {
            continue;
        }
        pl = crack_pipeline(user_input);
        print_pipeline(stdout, pl);
        if(pl->length == 0) {
            continue;
        }
        if(strcmp(pl->stage->argv[0], "cd") == 0) {
            if(pl->length > 1) {
                fprintf(stderr, "usage: cd <dir>\n");

            } else {
                chdir(pl->stage->argv[1]);
            }
            continue;
        }
        pipe(old);
        pipe(new);
        i = 0;
        while(i < pl->length) {
            /*printf("FORKING\n");*/
            pid = fork();
            if(pid < 0) {
                perror("pid");
                exit(10);
            }
            if(pid == 0) {
                /* no piping needed, one command */
                if(i == 0 && pl->length == 1) {
                    /*printf("RUNNING IN CHILD 0: %s\n", pl->stage->argv[0]);*/
                    close_pipes(old, new);
                    execute_command(pl->stage->argv);
                }
                /* first pipe */
                else if(i == 0) {
                    /*printf("RUNNING IN CHILD FIRST PIPE: %s\n", pl->stage->argv[0]);*/
                    if((dup2(old[WRITE], STDOUT_FILENO)) == -1) {
                        perror("dup2");
                        exit(13);
                    }
                    close_pipes(old, new);
                    execute_command(pl->stage->argv);
                }
                /* last pipe but only two commands */
                else if(i == pl->length - 1 && pl->length == 2) {
                    if((dup2(old[READ], STDIN_FILENO)) == -1) {
                        perror("dup2");
                        exit(13);
                    }
                    close_pipes(old, new);
                    execute_command(pl->stage->argv);
                }
                /* last pipe */
                else if(i == pl->length - 1) {
                    /*printf("RUNNING IN CHILD LAST PIPE: %s\n", pl->stage->argv[0]);*/
                    if((dup2(new[READ], STDIN_FILENO)) == -1) {
                        perror("dup2");
                        exit(14);
                    }
                    close_pipes(old, new);
                    execute_command(pl->stage->argv);
                }
                /* middle pipes */
                else {
                    /*printf("RUNNING IN CHILD MIDDLE PIPE: %s\n", pl->stage->argv[0]);*/
                    if((dup2(old[READ], STDIN_FILENO)) == -1) {
                        perror("dup2");
                        exit(15);
                    }
                    if((dup2(new[WRITE], STDOUT_FILENO)) == -1) {
                        perror("dup2");
                        exit(16);
                    }
                    close_pipes(old, new);
                    execute_command(pl->stage->argv);
                }
            }
            i++;
            pl->stage++;
            fflush(stdin);
        }
        close_pipes(old, new);
        for( ; i > 0; i--) {
            wait(NULL);
        }
    }
}

void sigquit_handle() {
    fflush(stdin);
    printf("\n");
    shell();
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigquit_handle;
    if((sigaction(SIGINT, &sa, NULL)) == -1) {
        perror("sigaction");
        exit(4);
    }
    shell();
    yylex_destroy();
    return 0;
}
