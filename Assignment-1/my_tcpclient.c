#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXN 100000     // The maximum size of the file being read
#define CHUNK_SIZE 50   // The chunk size with which we will send the file to the server
#define OUT_SIZE 200    // The size of the buffer used to store the output
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

    char buf[MAXN];
    memset(buf, 0, sizeof(buf));
    int n = read(fd, buf, MAXN);    // Read from the file into buffer
    if(n < 0) {
        perror("Unable to read file\n");
        exit(1);
    }
    buf[n++] = '.';
    buf[n] = '\0';
    close(fd);      // Close the file descriptor

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
    char file_buf[CHUNK_SIZE], out[OUT_SIZE];
    memset(file_buf, 0, sizeof(file_buf));
    memset(out, 0, sizeof(out));

    int next_ind = 0, sz;   // next_ind keeps track of the next index from which we need to read from the file to send to the server
    while(next_ind < strlen(buf)) {
        int i;
        for(i = 0; i < CHUNK_SIZE && next_ind + i < strlen(buf); i++) {
            file_buf[i] = buf[next_ind + i];
        }
        next_ind += i;
        sz = send(sockfd, file_buf, i, 0);      // Send CHUNK_SIZE (or whatever remains at the end) bytes to the server
        if(sz < 0) {
            perror("Unable to send to socket\n");
            exit(1);
        }
    }

    // Receive the number of characters, words and sentences from the server
    while(1) {
        char temp[OUT_SIZE];
        memset(temp, 0, sizeof(temp));
        sz = recv(sockfd, temp, OUT_SIZE, 0);
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