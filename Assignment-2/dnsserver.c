#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

#define MAXN 100        // Maximum size of buffer
#define IP_SIZE 20      // The size to store one IP address
#define PORT 20000      // Port number

int main() 
{
    int sockfd;     // Socket file descriptor
    struct sockaddr_in cli_addr, serv_addr;
    socklen_t clilen;

    // Create a UDP socket for the server, a negative return value indicates an error
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(1);
    }

    // Set the struct server address and client address to all zeros
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    memset(&cli_addr, 0, sizeof(cli_addr)); 

    serv_addr.sin_family = AF_INET;             // The family is AF_INET for the internet family
    serv_addr.sin_addr.s_addr = INADDR_ANY;     // Will accept conectins from any IP address
    serv_addr.sin_port = htons(PORT);           // The port number is converted to network byte order

    // Associate the socket with the correct port using the bind() system call
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Unable to bind local address\n");
        exit(1);
    }

    // buf stores the input from the client, out is used to send the required output back to the client
    char buf[MAXN], out[IP_SIZE];
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));

    while(1) {
        clilen = sizeof(cli_addr);
        printf("Server waiting for client connection\n");

        int n = recvfrom(sockfd, buf, MAXN, 0, (struct sockaddr *) &cli_addr, &clilen);
        if(n < 0) {
            perror("Unable to read from socket\n");
            exit(1);
        }
        if(n == 0) {
            printf("Client closed connection\n");
            break;
        }

        struct hostent *h = gethostbyname(buf);
        int sz;
        if(h == NULL) {
            strcpy(out, "0.0.0.0");
            sz = sendto(sockfd, out, strlen(out) + 1, 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
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
                sz = sendto(sockfd, out, strlen(out) + 1, 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
                if(sz < 0) {
                    perror("Unable to send to socket\n");
                    exit(1);
                }
                i++;
            }
        }
    }

    close(sockfd);      // Close the socket
    return 0;
}