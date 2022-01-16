// It is assumed that the input text file will end with a single full stop.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXN 100        // The maximum size of the file being read
#define CHUNK_SIZE 50   // The chunk size with which we will send the file to the server
#define PORT 20000      // The port number on which the server will be listening

int main(int argc, char *argv[])
{
    int sockfd, fd;
    struct sockaddr_in serv_addr;

    char *filename = argv[1];
    if((fd = open(filename, O_RDONLY)) < 0) {   // Open the file which has to be read
        perror("File not found\n");
        exit(1);
    }

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

    // The connect() system call establishes a connection with the server process
    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Unable to connect to server\n");
        exit(1);
    }

    // file_buf is used to send the file in chunks to the server, out stores the output received from the server
    char file_buf[CHUNK_SIZE], out[MAXN];
    memset(file_buf, 0, sizeof(file_buf));
    memset(out, 0, sizeof(out));

    int len = 0;
    do {
        len = read(fd, file_buf, CHUNK_SIZE);
        if(len > 0) {
            send(sockfd, file_buf, len, 0);
        }
    } while(len > 0);

    // It is assumed that the input text file will end with a single full stop.
    send(sockfd, ".", 1, 0);

    close(fd);

    // Receive the number of characters, words and sentences from the server
    int sz = 0;
    while(1) {
        char temp[MAXN];
        memset(temp, 0, sizeof(temp));
        sz = recv(sockfd, temp, MAXN, 0);
        if(sz < 0) {
            perror("Unable to read from socket\n");
            exit(1);
        }
        else if(sz == 0) {
            perror("Connection closed by server\n");
            exit(1);
        }
        strcat(out, temp);
        if(temp[sz - 1] == '\0') {
            break;
        }
    }
    
    printf("%s", out);      // Print the output received from the server

    close(sockfd);      // Close the socket
    return 0;
}