#include <stdio.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

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

    char file_buf[CHUNK_SIZE], out[MAXN];
    memset(file_buf, 0, sizeof(file_buf));
    memset(out, 0, sizeof(out));

    int len = 0;
    do {
        len = read(fd, file_buf, CHUNK_SIZE);
        if(len > 0) {
            sendto(sockfd, file_buf, len, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        }
    } while(len > 0);
    
    // It is assumed that the input text file will end with a single full stop.
    sendto(sockfd, ".", 1, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    close(fd);

    // Receive the number of characters, words and sentences from the server
    int sz = recvfrom(sockfd, out, MAXN, 0, NULL, NULL);
    if(sz < 0) {
        perror("Unable to read from socket\n");
        exit(1);
    }
    else if(sz == 0) {
        printf("Server closed the connection\n");
        exit(1);
    }
    
    printf("%s", out);      // Print the output received from the server

    close(sockfd);      // Close the socket
    return 0;
}