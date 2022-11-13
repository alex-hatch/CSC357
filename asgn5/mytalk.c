#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define BACKLOG 3

int vflag, aflag, nflag;
int cfd, sfd;

unsigned short convert_port(char *port) {
    int port_int;

    port_int = (int) strtol(port, NULL, 10);
    return htons(port_int);
}

void start_client(char *hostname, char *port) {
    struct hostent *he;
    struct sockaddr_in sa;

    he = gethostbyname(hostname);
    if (he == NULL) {
        herror("gethostbyname");
        exit(50);
    }

    if ((cfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(60);
    }

    if(vflag) {
        printf("Client socket created\n");
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *) he->h_addr));
    sa.sin_port = convert_port(port);
    sa.sin_family = PF_INET;
    /*printf("IP address: %s\n", inet_ntoa(*(struct in_addr *) he->h_addr));*/

    if ((connect(cfd, (const struct sockaddr *) &sa, sizeof(sa))) == -1) {
        perror("connect");
        exit(70);
    }

    printf("MADE IT HERE ClIENT\n");
}

void start_server(char *port) {
    char response[3];
    int accepted;
    struct sockaddr_in sa, peer;
    socklen_t peer_addr_size;
    char *deny_msg;

    memset(&sa, 0, sizeof(sa));
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");
    sa.sin_port = convert_port(port);
    sa.sin_family = PF_INET;

    if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(20);
    }

    if(vflag) {
        printf("Server socket created\n");
    }

    if ((bind(sfd, (const struct sockaddr *) &sa, sizeof(sa))) == -1) {
        perror("bind");
        exit(10);
    }

    if(vflag) {
        printf("Successful bind to port %s\n", port);
    }

    if ((listen(sfd, BACKLOG)) == -1) {
        perror("listen\n");
        exit(30);
    }

    if(vflag) {
        printf("Listening on port %s\n", port);
    }

    peer_addr_size = sizeof(struct sockaddr_in);

    accepted = accept(sfd, (struct sockaddr *) &peer, &peer_addr_size);
    if (accepted == -1) {
        perror("accept");
        exit(40);
    }

    deny_msg = "host denied connection request";
    printf("Mytalk request from [user]. Accept (y/n)?\n");
    fgets(response, sizeof(response) + 1, stdin);

    if (strcmp(response, "y\n") == 0 || strcmp(response, "yes") == 0) {
        printf("Connected!\n");
        send(cfd, "ok", 2, 0);
    } else {
        printf("Not accepted\n");
        send(cfd, deny_msg, strlen(deny_msg), 0);
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
                printf("hi\n");
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
        if(vflag) {
            printf("Starting server\n");
        }
        start_server(port);
    } else {
        /* client trying to connect to server */
        if(vflag) {
            printf("Client\n");
        }
        start_client(host_name, port);
    }

    return 0;
}
