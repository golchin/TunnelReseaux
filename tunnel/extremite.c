#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

void processRequest (int sock);

void ext_out(unsigned port, int tun0) {
    int pid;
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket()");
        exit(errno);
    }

    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt()");
        exit(errno);
    }

    struct sockaddr_in6 sin;

    sin.sin6_addr = in6addr_any;
    sin.sin6_family = AF_INET6;
    sin.sin6_port = htons((uint16_t) port);

    if(bind (sock, (struct sockaddr *) &sin, sizeof sin) < 0) {
        perror("bind()");
        exit(errno);
    }

    if(listen(sock, 5) < 0) {
        perror("listen()");
        exit(errno);
    }

    struct sockaddr_in csin = { 0 };
    socklen_t sinsize = sizeof(struct sockaddr_in);

    // char buff[256];
    while (1) {
        int csock = accept(sock, (struct sockaddr *) &csin, &sinsize);

        if(csock < 0) {
            perror("accept()");
            exit(errno);
        }

        printf("User accepted !\n");

        pid = fork();

        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0) {
            /* This is the client process */
            close(sock);
            processRequest(csock);
            exit(0);
        }
        else {
            close(csock);
        }

//        ssize_t lu = recv(csock, &buff, 255, 0);
//        if (lu > 0) {
//            buff[lu] = '\0';
//            printf("read (%d) : %s\n", (int) lu, buff);
//            write(tun0, &buff, (size_t) (lu + 1));
//        } else if (lu < 0) {
//            perror("recv()");
//            exit(errno);
//        }
    }

    printf("\n");

    //close(csock);
    //close(sock);
}

void ext_in(unsigned port, char *address, int tun0) {
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket()");
        exit(errno);
    }

    struct sockaddr_in6 sin;

    inet_pton(AF_INET6, address, &sin.sin6_addr);
    sin.sin6_family = AF_INET6;
    sin.sin6_port = htons((uint16_t) port);

    while (connect(sock,(struct sockaddr *) &sin, sizeof(struct sockaddr_in6)) < 0) {
        sleep(1);
    }

    printf("Connected !\n");

    char buff[256];
    while (1) {
        ssize_t lu = read(tun0, &buff, 255);
        if (lu > 0) {
            buff[lu] = '\0';
// alteration CRC           buff[lu-1] = '8';
            printf("read (%d) : %s\n", (int) lu, buff);
            send(sock, &buff, (size_t) lu, 0);
        } else if (lu < 0) {
            perror("recv()");
            exit(errno);
        }
    }

    close(sock);

}

void processRequest (int sock) {
    int n;
    char buffer[256];
    bzero(buffer,256);
    n = read(sock,buffer,255);

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("Here is the message: %s\n",buffer);
    n = write(sock,"I got your message",18);

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
}