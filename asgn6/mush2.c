#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>
#include "mush.h"

void execute_command(char **argv) {
    if((execvp(argv[0], argv)) == -1) {
        perror("execvp");
        _exit(2);
    }
}

void shell() {
    int pid;
    int i;
    char *user_input;
    struct pipeline *pl;
    int fd[2];

    while(1) {
        printf("8-P ");
        user_input = readLongString(stdin);
        if(feof(stdin)) {
            yylex_destroy();
            free(user_input);
            exit(0);
        }
        pl = crack_pipeline(user_input);
        pipe(fd);
        i = 0;
        while(i < pl->length) {
            pid = fork();
            if(pid < 0) {
                perror("pid");
                exit(10);
            }
            if(pid == 0) {
                if(i == 0 && pl->length == 1) {
                    execute_command(pl->stage->argv);
                }
                if(i == 0) {
                    dup2(fd[1], STDOUT_FILENO);
                    if((close(fd[0]) == -1)) {
                        perror("close1");
                        exit(11);
                    }
                    if((close(fd[1]) == -1)) {
                        perror("close2");
                        exit(12);
                    }
                    execute_command(pl->stage->argv);
                }
                else if(i == pl->length - 1) {
                    dup2(fd[0], STDIN_FILENO);
                    if((close(fd[0]) == -1)) {
                        perror("close3");
                        exit(13);
                    }
                    if((close(fd[1]) == -1)) {
                        perror("close4");
                        exit(14);
                    }
                    execute_command(pl->stage->argv);
                } else {
                    dup2(fd[0], STDIN_FILENO);
                    dup2(fd[1], STDOUT_FILENO);
                    if((close(fd[0]) == -1)) {
                        perror("close5");
                        exit(13);
                    }
                    if((close(fd[1]) == -1)) {
                        perror("close6");
                        exit(14);
                    }
                    execute_command(pl->stage->argv);
                }
            }
            if(i == 0) {
                printf("RUNNING\n");
                if((close(fd[0]) == -1)) {
                    perror("close7");
                    exit(15);
                }
                if((close(fd[1]) == -1)) {
                    perror("close8");
                    exit(16);
                }
            }
            i++;
            pl->stage++;
        }
        wait(NULL);
    }
}

void sigquit_handle() {
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
