#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXN 105
#define PORT 20000

int main(int argc, char *argv[]) 
{
    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in cli_addr, serv_addr;

    char buf[MAXN];
    memset(buf, 0, sizeof(buf));

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr)); 
    memset(&cli_addr, 0, sizeof(cli_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Unable to bind local address\n");
        exit(1);
    }

    listen(sockfd, 5);

    while(1) {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0) {
            perror("Unable to accept connection\n");
            exit(1);
        }

        int num_sentences = 0, num_words = 0, num_chars = 0;
        char last_char;
        while(1) {
            int n = recv(newsockfd, buf, MAXN, 0);
            if(n < 0) {
                perror("Unable to read from socket\n");
                exit(1);
            }
            if(n == 0) {
                printf("Client closed connection\n");
                break;
            }

            // buf[n] = '\0';
            // printf("%s\n", buf);
            for(int i = 0; i < n; i++) {
                // process these characters
            }
        }

        // send the results to the client

        close(newsockfd);
    }

    close(sockfd);

    return 0;
}