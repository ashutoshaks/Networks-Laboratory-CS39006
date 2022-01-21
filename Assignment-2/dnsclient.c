#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

#define MAXN 100        // The maximum size of the file being read
#define IP_SIZE 20      // The size to store one IP address
#define PORT 20000      // The port number on which the server will be listening

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;

    // Create a TCP socket for the client
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(1);
    }

    // Initialize the server address to all zeros
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;     // The family is AF_INET for the internet family
    serv_addr.sin_port = htons(PORT);   // The port number is converted to network byte order

    // We assume that the server is running on the same machine as the client. 127.0.0.1 is a special address for "localhost" (this machine)
    int s = inet_aton("127.0.0.1", &serv_addr.sin_addr);
    if(s == 0) {
        printf("Not in presentation format\n");
        exit(1);
    }
    else if(s < 0) {
        perror("Unable to convert IP address\n");
        exit(1);
    }

    char buf[MAXN], out[IP_SIZE];
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));

    printf("Enter the DNS name:\n");
    scanf("%s", buf);
    sendto(sockfd, buf, strlen(buf) + 1, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(sockfd, &fd);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    int received = 0;

    while(1) {
        int ret = select(sockfd + 1, &fd, NULL, NULL, &tv);
        if(ret < 0) {
            perror("Unable to select\n");
            exit(1);
        }

        if(FD_ISSET(sockfd, &fd)) {
            memset(out, 0, sizeof(out));
            int n = recvfrom(sockfd, out, IP_SIZE, 0, NULL, NULL);
            if(n < 0) {
                perror("Unable to read from socket\n");
                exit(1);
            }
            if(strcmp(out, "0.0.0.0") == 0) {
                printf("\nNo such host found\n");
                exit(1);
            }
            if(received == 0) {
                printf("\nIP address(es) for %s:\n", buf);
            }
            printf("%s\n", out);
            received = 1;
        }
        else {
            close(sockfd);
            if(received == 0) {
                printf("\nTimed out in 2 sec - No response from server\n");
                exit(1);
            }
            else {
                exit(0);
            }
        }
    }

    close(sockfd);
    return 0;
}