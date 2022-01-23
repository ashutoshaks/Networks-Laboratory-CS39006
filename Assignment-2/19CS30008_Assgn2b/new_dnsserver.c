#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

#define MAXN 100            // Maximum size of buffer
#define IP_SIZE 20          // The size to store one IP address
#define PORT 20000          // The port number

int max(int a, int b) {
    return ((a > b) ? a : b);
}

void process_tcp(int new_tcp_sockfd) {
    char buf[MAXN], temp[MAXN], out[IP_SIZE];
    memset(buf, 0, sizeof(buf));
    memset(temp, 0, sizeof(temp));
    memset(out, 0, sizeof(out));

    while(1) {
        memset(temp, 0, sizeof(temp));
        int n = recv(new_tcp_sockfd, temp, MAXN, 0);
        if(n < 0) {
            perror("Unable to read from socket\n");
            exit(1);
        }
        if(n == 0) {
            printf("Client closed connection\n");
            return;
        }
        strcat(buf, temp);
        if(temp[n - 1] == '\0') {
            break;
        }
    }

    struct hostent *h = gethostbyname(buf);
    int sz;
    if(h == NULL) {
        strcpy(out, "0.0.0.0");
        sz = send(new_tcp_sockfd, out, strlen(out) + 1, 0);
        if(sz < 0) {
            perror("Unable to send to socket\n");
            exit(1);
        }
    }
    else {
        int i = 0;
        while(h->h_addr_list[i] != NULL) {
            memset(out, 0, sizeof(out));
            strcpy(out, inet_ntoa(*((struct in_addr *)h->h_addr_list[i])));
            sz = send(new_tcp_sockfd, out, strlen(out) + 1, 0);
            if(sz < 0) {
                perror("Unable to send to socket\n");
                exit(1);
            }
            i++;
        }
        sz = send(new_tcp_sockfd, "\0", 1, 0);
        if(sz < 0) {
            perror("Unable to send to socket\n");
            exit(1);
        }
    }
}

void process_udp(int udp_sockfd) {
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    memset(&cli_addr, 0, sizeof(cli_addr));

    char buf[MAXN], out[IP_SIZE];
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));

    int n = recvfrom(udp_sockfd, buf, MAXN, 0, (struct sockaddr *) &cli_addr, &clilen);
    if(n < 0) {
        perror("Unable to read from socket\n");
        exit(1);
    }
    if(n == 0) {
        printf("Client closed connection\n");
        return;
    }

    struct hostent *h = gethostbyname(buf);
    int sz;
    if(h == NULL) {
        strcpy(out, "0.0.0.0");
        sz = sendto(udp_sockfd, out, strlen(out) + 1, 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
        if(sz < 0) {
            perror("Unable to send to socket\n");
            exit(1);
        }
    }
    else {
        int i = 0;
        while(h->h_addr_list[i] != NULL) {
            memset(out, 0, sizeof(out));
            strcpy(out, inet_ntoa(*((struct in_addr *)h->h_addr_list[i])));
            sz = sendto(udp_sockfd, out, strlen(out) + 1, 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
            if(sz < 0) {
                perror("Unable to send to socket\n");
                exit(1);
            }
            i++;
        }
    }
}

int main() 
{
    int tcp_sockfd, new_tcp_sockfd, udp_sockfd;     // Socket file descriptor
    struct sockaddr_in serv_addr;

    // Create a TCP socket for the server, a negative return value indicates an error
    if((tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create TCP socket\n");
        exit(1);
    }

    // Create a UDP socket for the server, a negative return value indicates an error
    if((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Unable to create UDP socket\n");
        exit(1);
    }

    // Set the struct server address and client address to all zeros
    memset(&serv_addr, 0, sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;             // The family is AF_INET for the internet family
    serv_addr.sin_addr.s_addr = INADDR_ANY;     // Will accept conectins from any IP address
    serv_addr.sin_port = htons(PORT);           // The port number is converted to network byte order

    // Associate the socket with the correct port using the bind() system call
    if(bind(tcp_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Unable to bind local address\n");
        exit(1);
    }

    // Associate the socket with the correct port using the bind() system call
    if(bind(udp_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Unable to bind local address\n");
        exit(1);
    }

    // Specifies that up to 5 concurrent client requests will be queued up while the system is executing the "accept" system call below
    listen(tcp_sockfd, 5);

    char buf[MAXN], out[IP_SIZE];
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));

    while(1) {
        printf("Server waiting for client connection\n");

        fd_set fd;
        FD_ZERO(&fd);
        FD_SET(tcp_sockfd, &fd);
        FD_SET(udp_sockfd, &fd);

        int ret = select(max(tcp_sockfd, udp_sockfd) + 1, &fd, NULL, NULL, NULL);
        if(ret < 0) {
            perror("Unable to select\n");
            exit(1);
        }

        if(FD_ISSET(tcp_sockfd, &fd)) {     // Data in TCP socket
            struct sockaddr_in cli_addr;
            socklen_t clilen = sizeof(cli_addr);
            memset(&cli_addr, 0, sizeof(cli_addr));

            new_tcp_sockfd = accept(tcp_sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if(new_tcp_sockfd < 0) {
                perror("Unable to accept connection\n");
                exit(1);
            }
            if(fork() == 0) {       // Child process
                close(tcp_sockfd);
                close(udp_sockfd);
                process_tcp(new_tcp_sockfd);
                close(new_tcp_sockfd);
                exit(0);
            }
            close(new_tcp_sockfd);
        }

        if(FD_ISSET(udp_sockfd, &fd)) {     // Data in UDP socket
            process_udp(udp_sockfd);
        }
    }

    close(udp_sockfd);      // Close the socket
    return 0;
}