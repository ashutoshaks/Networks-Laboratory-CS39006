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
#define TCP_PORT 20000      // TCP port number
#define UDP_PORT 20001      // UDP port number

int main() 
{
    int tcp_sockfd, udp_sockfd;     // Socket file descriptor
    struct sockaddr_in cli_addr, tcp_serv_addr, udp_serv_addr;
    socklen_t clilen;

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
    memset(&tcp_serv_addr, 0, sizeof(tcp_serv_addr)); 
    memset(&udp_serv_addr, 0, sizeof(udp_serv_addr));
    memset(&cli_addr, 0, sizeof(cli_addr)); 

    tcp_serv_addr.sin_family = AF_INET;             // The family is AF_INET for the internet family
    tcp_serv_addr.sin_addr.s_addr = INADDR_ANY;     // Will accept conectins from any IP address
    tcp_serv_addr.sin_port = htons(TCP_PORT);       // The port number is converted to network byte order

    // Associate the socket with the correct port using the bind() system call
    if(bind(tcp_sockfd, (struct sockaddr *) &tcp_serv_addr, sizeof(tcp_serv_addr)) < 0) {
        perror("Unable to bind local address\n");
        exit(1);
    }

    udp_serv_addr.sin_family = AF_INET;             // The family is AF_INET for the internet family
    udp_serv_addr.sin_addr.s_addr = INADDR_ANY;     // Will accept conectins from any IP address
    udp_serv_addr.sin_port = htons(UDP_PORT);       // The port number is converted to network byte order

    // Associate the socket with the correct port using the bind() system call
    if(bind(udp_sockfd, (struct sockaddr *) &udp_serv_addr, sizeof(udp_serv_addr)) < 0) {
        perror("Unable to bind local address\n");
        exit(1);
    }

    // Specifies that up to 5 concurrent client requests will be queued up while the system is executing the "accept" system call below
    listen(tcp_sockfd, 5);

    // buf stores the input from the client, out is used to send the required output back to the client
    char buf[MAXN], out[IP_SIZE];
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));

    while(1) {
        clilen = sizeof(cli_addr);
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

        if(FD_ISSET(tcp_sockfd, &fd)) {
            // accept first
        }

        if(FD_ISSET(udp_sockfd, &fd)) {
            if(fork() == 0) {
                
            }
            
        }
    }

    close(udp_sockfd);      // Close the socket
    return 0;
}