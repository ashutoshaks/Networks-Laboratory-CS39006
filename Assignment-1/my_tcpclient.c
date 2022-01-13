#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXN 100000
#define CHUNK_SIZE 50
#define OUT_SIZE 100
#define PORT 20000

int main(int argc, char *argv[])
{
    int sockfd, fd;
    struct sockaddr_in serv_addr;

    char *filename = argv[1];
    if((fd = open(filename, O_RDONLY)) < 0) {
        perror("File not found\n");
        exit(1);
    }

    char buf[MAXN];
    memset(buf, 0, sizeof(buf));
    int n = read(fd, buf, MAXN);
    if(n < 0) {
        perror("Unable to read file\n");
        exit(1);
    }
    buf[n] = '\0';
    close(fd);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    int s = inet_aton("127.0.0.1", &serv_addr.sin_addr);
    if(s == 0) {
        printf("Not in presentation format\n");
        exit(1);
    }
    else if(s < 0) {
        perror("Unable to convert IP address\n");
        exit(1);
    }

    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Unable to connect to server\n");
        exit(1);
    }

    char file_buf[CHUNK_SIZE], sentences[OUT_SIZE], words[OUT_SIZE], chars[OUT_SIZE];
    memset(file_buf, 0, sizeof(file_buf));
    memset(sentences, 0, sizeof(sentences));
    memset(words, 0, sizeof(words));
    memset(chars, 0, sizeof(chars));

    int next_ind = 0;
    while(next_ind < strlen(buf)) {
        int i;
        for(i = 0; i < CHUNK_SIZE && next_ind + i < strlen(buf); i++) {
            file_buf[i] = buf[next_ind + i];
        }
        next_ind += i;
        send(sockfd, file_buf, i, 0);
    }

    // get the results from server and display them

    close(sockfd);
    return 0;
}