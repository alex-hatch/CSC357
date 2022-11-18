#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <time.h>
#include "talk.h"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pwd.h>
#include <signal.h>

#define BACKLOG 1
#define MSG_SIZE 1000
#define TIMEOUT 100

int vflag, aflag, nflag;
int fd;
int accept_fd;

/* close client when SIGINT received */
void client_close() {
    char *msg = "\nConnection closed. ^C to terminate\n";
    
    /* Notify server of exit */
    if ((send(fd, msg, strlen(msg), 0)) == -1) {
        perror("send");
        exit(205);
    }
    close(fd);
    close(accept_fd);
    stop_windowing();
    exit(0);
}

/* close server when SIGINT received */
void server_close() {
    char *msg = "\nConnection closed. ^C to terminate\n";
    
    /* Notify client of exit */
    if ((send(accept_fd, msg, strlen(msg), 0)) == -1) {
        perror("send");
        exit(205);
    }
    close(fd);
    close(accept_fd);
    stop_windowing();
    exit(0);
}

unsigned short convert_port(char *port) {
    int port_int;
    port_int = (int) strtol(port, NULL, 10);
    return htons(port_int);
}

int message_length(char *str) {
    int count;
    count = 0;
    while (*str) {
        count++;
        str++;
    }
    return count;
}

void client_chat() {
    char *server_msg;
    char *client_msg;
    struct pollfd pfd;
    struct pollfd input;
    struct sigaction sig;
    ssize_t size_read;
    memset(&pfd, 0, sizeof(pfd));
    memset(&input, 0, sizeof(input));
    pfd.fd = fd;
    pfd.events = POLLIN;
    input.fd = 0;
    input.events = POLLIN;


    if (!nflag) {
        start_windowing();
        set_verbosity(1);
    }

    /* install signal handling */
    sig.sa_handler = client_close;
    sigaction(SIGINT, &sig, NULL);

    while (1) {
        /* listen for server messages in a non-blocking fashion. */
        if (poll(&pfd, 1, TIMEOUT) == -1) {
            perror("poll");
            exit(404);
        }
        /* if there is a message to read, read it. */
        if (pfd.revents & POLLIN) {
            server_msg = calloc(MSG_SIZE, 1);
            if ((size_read = recv(fd, server_msg, MSG_SIZE, 0)) == -1) {
                perror("recv");
                exit(200);
            }
            /* display message */
            write_to_output(server_msg, message_length(server_msg));
            free(server_msg);

        } else {
            /* listen for client messages in a non-blocking fashion. */
            if (poll(&input, 1, 100) == -1) {
                perror("poll");
                exit(402);
            }
            /* if there is a message to write, write it. */
            if (input.revents & POLLIN) {
                client_msg = calloc(MSG_SIZE, 1);
                read_from_input(client_msg, MSG_SIZE);
                if ((send(fd, client_msg, MSG_SIZE, 0)) == -1) {
                    perror("send");
                    exit(205);
                }
                free(client_msg);
                fflush(stdin);
            }
        }
    }
}

void server_chat() {
    char *client_msg;
    char *server_msg;
    ssize_t size_read;
    struct pollfd pfd;
    struct pollfd input;
    struct sigaction sig;

    memset(&pfd, 0, sizeof(pfd));
    memset(&input, 0, sizeof(input));
    pfd.fd = accept_fd;
    pfd.events = POLLIN;
    input.fd = 0;
    input.events = POLLIN;

    if (!nflag) {
        start_windowing();
        set_verbosity(1);
    }
    
    /* install signal handling */
    sig.sa_handler = server_close;
    sigaction(SIGINT, &sig, NULL);

    while (1) {
        /* listen for client messages in a non-blocking fashion */
        if (poll(&pfd, 1, 100) == -1) {
            perror("poll");
            exit(400);
        }
        /* if there is a message to read, read it. */
        if (pfd.revents & POLLIN) {
            client_msg = calloc(MSG_SIZE, 1);
            if ((size_read = recv(accept_fd, client_msg, MSG_SIZE, 0)) == -1) {
                perror("send");
                exit(201);
            }
            write_to_output(client_msg, message_length(client_msg));
            free(client_msg);
        } else {
            /* listen for server messages in a non-blocking fashion */
            if (poll(&input, 1, 100) == -1) {
                perror("poll");
                exit(403);
            }
            if (input.revents & POLLIN) {
                server_msg = calloc(MSG_SIZE, 1);
                read_from_input(server_msg, MSG_SIZE);
                if ((send(accept_fd, server_msg, MSG_SIZE, 0)) == -1) {
                    perror("send");
                    exit(203);
                }
                free(server_msg);
                fflush(stdin);
            }
        }
    }
}

char *get_user(uid_t uid) {
    struct passwd *pws;
    pws = getpwuid(uid);
    return pws->pw_name;
}

void start_client(char *hostname, char *port) {
    struct hostent *he;
    struct sockaddr_in sa;
    struct pollfd *pollfd;
    char *user_address;
    char *server_response;
    pollfd = malloc(sizeof(struct pollfd));
    pollfd->fd = fd;
    pollfd->events = POLLIN;

    /* figure out where to connect to */
    he = gethostbyname(hostname);
    if (he == NULL) {
        herror("gethostbyname");
        exit(50);
    }

    /* initialize a socket */
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(60);
    }

    if (vflag) {
        printf("Client socket created\n");
    }

    /* clear the sock adder and fill */
    memset(&sa, 0, sizeof(sa));
    sa.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *) he->h_addr));
    sa.sin_port = convert_port(port);
    sa.sin_family = AF_INET;
    
    /* connect to server */
    if ((connect(fd, (const struct sockaddr *) &sa, sizeof(sa))) == -1) {
        perror("connect");
        exit(70);
    }

    /* send the server who is trying to connect. */
    user_address = get_user(getuid());
    if ((send(fd, user_address, 1000, 0)) == -1) {
        perror("send");
        exit(245);
    }
    
    free(user_address);
    /* handshake with the server to connect */
    if (!aflag) {
        printf("Waiting for response from %s\n", hostname);
        server_response = malloc(100);
        if ((recv(fd, server_response, 100, 0)) == -1) {
            perror("recv");
            exit(432);
        }
        if (strcmp(server_response, "ok") == 0) {
            printf("Server accepted connection\n");
            free(server_response);
            client_chat();
        } else {
            printf("%s denied connection\n", hostname);
        }
    } else {
        client_chat();
    }
}

void start_server(char *port) {
    char response[4];
    struct sockaddr_in sa;
    socklen_t sa_addr_size;
    struct hostent *hostent;
    char *client_request;
    
    /* clear and set up the sockaddr */
    memset(&sa, 0, sizeof(sa));
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = convert_port(port);
    sa.sin_family = AF_INET;

    
    /* initialize the socket */
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(20);
    }

    if (vflag) {
        printf("Server socket created\n");
    }

    /* bind the sockaddr */
    if ((bind(fd, (const struct sockaddr *) &sa, sizeof(sa))) == -1) {
        perror("bind");
        exit(10);
    }

    if (vflag) {
        printf("Successful bind to port %s\n", port);
    }

    /* listen for connections to the server */
    if ((listen(fd, BACKLOG)) == -1) {
        perror("listen\n");
        exit(30);
    }

    if (vflag) {
        printf("Listening on port %s\n", port);
    }

    /* accept any connections */
    sa_addr_size = sizeof(struct sockaddr_in);
    accept_fd = accept(fd, (struct sockaddr *) &sa, &sa_addr_size);
    if (accept_fd == -1) {
        perror("accept");
        exit(40);
    }

    /* figure out what server the client is connecting from */
    hostent = gethostbyaddr(&(sa.sin_addr), sizeof(sa.sin_addr), AF_INET);
    if (hostent == NULL) {
        perror("hostent");
        exit(492);
    }

    /* handshake with the client to establish a valid connection */
    if (!aflag) {
        client_request = malloc(100);
        if ((recv(accept_fd, client_request, 100, 0)) == -1) {
            perror("recv");
            exit(237);
        }
        printf("Mytalk request from %s@%s. Accept (y/n)?\n", client_request, hostent->h_name);
        free(client_request);
        fgets(response, sizeof(response) + 1, stdin);
        if (strcmp(response, "y\n") == 0 || strcmp(response, "yes\n") == 0) {
            if ((send(accept_fd, "ok", 2, 0)) == -1) {
                perror("send");
                exit(532);
            }
            server_chat();
        } else {
            printf("Not accepted\n");
        }
    } else {

        /* automatically accept the connection */
        client_request = malloc(100);
        if ((recv(accept_fd, client_request, 100, 0)) == -1) {
            perror("recv");
            exit(237);
        }
        free(client_request);
        if ((send(accept_fd, "ok", 2, 0)) == -1) {
            perror("send");
            exit(532);
        }

        server_chat();
    }

}

int main(int argc, char *argv[]) {
    int c;
    char *host_name;
    char *port;
    vflag = aflag = nflag = opterr = 0;
    host_name = port = NULL;

    while ((c = getopt(argc, argv, "vaN")) != -1) {
        switch (c) {
            case 'N':
                nflag = 1;
                break;
            case 'v':
                vflag = 1;
                break;
            case 'a':
                aflag = 1;
                break;
            default:
                printf("usage: mytalk [ -v ] [ -a ] [ -N ] [ hostname ] port\n");
        }
    }

    if (argc - optind == 1) {
        port = strdup(argv[optind]);
    } else if (argc - optind == 2) {
        host_name = strdup(argv[optind]);
        port = strdup(argv[optind + 1]);
    }

    if (host_name == NULL) {
        /* start the server */
        if (vflag) {
            printf("Starting server\n");
        }
        start_server(port);
    } else {
        /* client trying to connect to server */
        if (vflag) {
            printf("Starting client connection to server\n");
        }
        start_client(host_name, port);
    }

    return 0;
}
