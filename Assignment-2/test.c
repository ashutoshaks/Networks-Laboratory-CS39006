#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

#define MAXN 100        // The maximum size of the file being read
#define IP_SIZE 20      // The size to store one IP address
#define PORT 20001      // The port number on which the server will be listening

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;

    char buf[MAXN];
    memset(buf, 0, sizeof(buf));

    printf("Enter the DNS name:\n");
    scanf("%s", buf);

    // Create a TCP socket for the client
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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

    // Make the socket non-blocking
    int args = fcntl(sockfd, F_GETFL);
    args |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, args);

    int ret = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if(ret < 0) {
        if(errno == EINPROGRESS) {
            struct timeval tv;
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            fd_set fd;
            FD_ZERO(&fd);
            FD_SET(sockfd, &fd);
            ret = select(sockfd + 1, NULL, &fd, NULL, &tv);
            if(ret > 0) {
                socklen_t len = sizeof(int);
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &ret, (socklen_t *) &len);
                if(ret != 0) {
                    printf("hi\n");
                    perror("Unable to connect\n");
                    exit(1);
                }
            }
            else {
                printf("\nTimed out in 2 sec - No response from server\n");
                exit(1);
            }
        }
        else {
            printf("ho\n");
            perror("Unable to connect");
            exit(1);
        }
    }

    // Make the socket blocking again
    args &= (~O_NONBLOCK);
    fcntl(sockfd, F_SETFL, args);

    send(sockfd, buf, strlen(buf) + 1, 0);

    char out[IP_SIZE], temp[IP_SIZE];
    memset(out, 0, sizeof(out));
    memset(temp, 0, sizeof(temp));

    int received = 0, ind = 0, end = 0;
    char prev = 'a';

    while(!end) {
        memset(temp, 0, sizeof(temp));
        int n = recv(sockfd, temp, IP_SIZE, 0);
        if(n < 0) {
            perror("Unable to read from socket\n");
            exit(1);
        }
        for(int i = 0; i < n; i++) {
            out[ind++] = temp[i];
            if(temp[i] == '\0') {
                if(prev == '\0') {
                    end = 1;
                    break;
                }
                if(strcmp(out, "0.0.0.0") == 0) {
                    printf("\nNo such host found\n");
                    exit(1);
                }
                if(received == 0) {
                    printf("\nIP address(es) for %s:\n", buf);
                }
                printf("%s\n", out);
                memset(out, 0, sizeof(out));
                ind = 0;
                received = 1;
            }
            prev = temp[i];
        }
    }

    close(sockfd);
    return 0;
}