#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <time.h>

#define BACKLOG 3

int vflag, aflag, nflag;
int fd;
int accept_fd;

unsigned short convert_port(char *port) {
    int port_int;

    port_int = (int) strtol(port, NULL, 10);
    return htons(port_int);
}

void client_chat() {
    char *server_msg;
    char *client_msg;
    while(1) {
        server_msg = malloc(100);
        if ((recv(fd, server_msg, 100, 0) == -1)) {
            perror("recv");
            exit(200);
        }
        printf("server: %s", server_msg);
        free(server_msg);

        printf(">>>");
        client_msg = malloc(100);
        fgets(client_msg, 100, stdin);
        if((send(fd, client_msg, 100, 0)) == -1) {
            perror("send");
            exit(205);
        }
        free(client_msg);
    }
}

void server_chat() {
    char *client_msg;
    char *server_msg;
    while(1) {
        server_msg = malloc(100);
        printf(">>>");
        fgets(server_msg, 100, stdin);
        if((send(accept_fd, server_msg, 100, 0)) == -1) {
            perror("send");
            exit(203);
        }
        free(server_msg);

        client_msg = malloc(100);
        if ((recv(accept_fd, client_msg, 100, 0) == -1)) {
            perror("send");
            exit(201);
        }
        printf("client: %s", client_msg);
        free(client_msg);
    }

}

void start_client(char *hostname, char *port) {
    struct hostent *he;
    struct sockaddr_in sa;
    struct pollfd *pollfd;
    pollfd = malloc(sizeof(struct pollfd));
    pollfd->fd = fd;
    pollfd->events = POLLIN;

    he = gethostbyname(hostname);
    if (he == NULL) {
        herror("gethostbyname");
        exit(50);
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(60);
    }

    if (vflag) {
        printf("Client socket created\n");
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *) he->h_addr));
    sa.sin_port = convert_port(port);
    sa.sin_family = AF_INET;
    /*printf("IP address: %s\n", inet_ntoa(*(struct in_addr *) he->h_addr));*/
    if ((connect(fd, (const struct sockaddr *) &sa, sizeof(sa))) == -1) {
        perror("connect");
        exit(70);
    }

    client_chat();
}

void start_server(char *port) {
    char response[4];
    struct sockaddr_in sa;
    socklen_t sa_addr_size;
    char *deny_msg;

    memset(&sa, 0, sizeof(sa));
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");
    sa.sin_port = convert_port(port);
    sa.sin_family = AF_INET;


    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(20);
    }

    if (vflag) {
        printf("Server socket created\n");
    }

    if ((bind(fd, (const struct sockaddr *) &sa, sizeof(sa))) == -1) {
        perror("bind");
        exit(10);
    }

    if (vflag) {
        printf("Successful bind to port %s\n", port);
    }

    if ((listen(fd, BACKLOG)) == -1) {
        perror("listen\n");
        exit(30);
    }

    if (vflag) {
        printf("Listening on port %s\n", port);
    }

    sa_addr_size = sizeof(struct sockaddr_in);

    accept_fd = accept(fd, (struct sockaddr *) &sa, &sa_addr_size);
    if (accept_fd == -1) {
        perror("accept");
        exit(40);
    }


    deny_msg = "host denied connection request";
    printf("Mytalk request from [user]. Accept (y/n)?\n");
    fgets(response, sizeof(response) + 1, stdin);

    if (strcmp(response, "y\n") == 0 || strcmp(response, "yes\n") == 0) {
        printf("Connected!\n");
        server_chat();
    } else {
        printf("Not accepted\n");
        if((send(fd, deny_msg, strlen(deny_msg), 0)) == -1) {
            perror("send");
        }
        exit(80);
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

    /*
    printf("-N: %d\n", nflag);
    printf("-v: %d\n", vflag);
    printf("-a: %d\n", aflag);
    printf("port: %s\n", port);
    printf("host: %s\n", host_name);
     */

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
