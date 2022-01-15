#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXN 100        // Maximum size of buffer
#define OUT_SIZE 200    // Maximum size of output
#define PORT 20000      // Port number

// Function to check if a character is alphanumeric
int isAlphaNum(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
}

int main(int argc, char *argv[]) 
{
    int sockfd, newsockfd;      // Socket file descriptors
    int clilen;
    struct sockaddr_in cli_addr, serv_addr;

    // buf stores the input from the client, out is used to send the required output back to the client
    char buf[MAXN], out[OUT_SIZE];
    memset(buf, 0, sizeof(buf));

    // Create a TCP socket for the server, a negative return value indicates an error
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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

    // Specifies that up to 5 concurrent client requests will be queued up while the system is executing the "accept" system call below
    listen(sockfd, 5);

    while(1) {
        clilen = sizeof(cli_addr);

        // The accept system call blocks until a client connects to the server
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0) {
            perror("Unable to accept connection\n");
            exit(1);
        }

        int num_sentences = 0, num_words = 0, num_chars = 0, sz;
        int end = 0;    // Flag to indicate whether we have reached the end of the input file
        char last_char = '\0';      // Stores the last character read

        while(!end) {
            int n = recv(newsockfd, buf, MAXN, 0);  // Receive a message from the socket
            if(n < 0) {
                perror("Unable to read from socket\n");
                exit(1);
            }
            if(n == 0) {
                printf("Client closed connection\n");
                break;
            }

            // Process the characters received during the last call to recv()
            for(int i = 0; i < n; i++) {
                if(isAlphaNum(buf[i])) {
                    // if the present character is alphanumeric and the last character was not alphanumeric, we increment the number of words
                    if(!isAlphaNum(last_char)) {
                        num_words++;
                    }

                    // keep moving ahead till we are in the same word
                    while(i + 1 < n && isAlphaNum(buf[i + 1])) {
                        last_char = buf[i];
                        i++;
                    }
                }
                if(buf[i] == '.') {
                    if(last_char == '.') {
                        end = 1;    // Two consecutive full stops indicates the end of the input
                        break;
                    }
                    num_sentences++;    // Increment the number of sentences whenever there is a full stop (except for the last full stop)
                }
                last_char = buf[i];     // Store the last character read
            }
            num_chars += n;     // Update the number of characters read
        }
        num_chars--;    // to account for the extra '.' at the end of the input

        // send the results to the client
        memset(out, 0, sizeof(out));
        sprintf(out, "Number of characters: %d\nNumber of words: %d\nNumber of sentences: %d\n", num_chars, num_words, num_sentences);
        sz = send(newsockfd, out, strlen(out) + 1, 0);
        if(sz < 0) {
            perror("Unable to send to socket\n");
            exit(1);
        }
        close(newsockfd);   // Close the socket newsockfd
    }

    close(sockfd);      // Close the socket sockfd
    return 0;
}