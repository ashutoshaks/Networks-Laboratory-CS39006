#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>

#define ERROR(msg, ...) printf("\033[1;31m[ERROR] "msg" \033[0m\n", ##__VA_ARGS__);
#define SUCCESS(msg, ...) printf("\033[1;36m[INFO] "msg" \033[0m\n", ##__VA_ARGS__);
#define DEBUG(msg, ...) printf("\033[1;34m[DEBUG] "msg" \033[0m\n", ##__VA_ARGS__);
#define PROMPT(msg, ...) printf("\033[1;32m"msg" \033[0m", ##__VA_ARGS__);

#define COMMAND_SIZE 1000
#define MAX_SIZE 1000

const char* code_200 = "200";
const char* code_500 = "500";
const char* code_600 = "600";

void sendFile(int sockfd, int fd) {
    struct stat st;
    fstat(fd, &st);
    long int file_size = st.st_size;

    DEBUG("File size: %ld", file_size);

    long int len = 0;
    int sz = MAX_SIZE - 3;
    char buf[MAX_SIZE];
    char file_buf[sz];
    do {
        short n = read(fd, file_buf, sz);
        DEBUG("Read %d bytes", n);
        if (len + n < file_size) {
            buf[0] = 'M';
        } else {
            buf[0] = 'L';
        }
        memcpy(buf + 1, n, 2);
        memcpy(buf + 3, file_buf, n);

        send(sockfd, buf, n + 3, 0);
        len += n;
    } while (len < file_size);
}

void recvFile(int sockfd, int fd) {
    char buf[MAX_SIZE];
    short rem = 0;
    int type = 1, len = 0, data = 0;
    char curr_type;
    char temp_len[2];
    int done = 0;
    do {
        int n = recv(sockfd, buf, MAX_SIZE, 0);
        DEBUG("Received %d bytes", n);
        for (int i = 0; i < n; i++) {
            if (type) {
                curr_type = buf[i];
                type = 0; len = 1; data = 0;
            } else if (len == 1) {
                temp_len[0] = buf[i];
                len = 2;
            } else if (len == 2) {
                temp_len[1] = buf[i];
                len = 0; data = 1;
                rem = ntohs(*(short*)temp_len);
            } else {
                int can_write = ((rem < n - i) ? rem : n - i);
                write(fd, buf + i, can_write);
                DEBUG("Wrote %d bytes", can_write);
                rem -= can_write;
                i += can_write - 1;
                if (rem == 0) {
                    if (curr_type == 'L') {
                        done = 1;
                    }
                    data = 0; type = 1;
                }
            }
        }
    } while (!done);
}

int recieve(int sockfd, char* buf, int max_size, char delim) {
    memset(buf, 0, sizeof(buf));
    char temp[max_size];
    int tot = 0;
    while(1) {
        memset(temp, 0, sizeof(temp));
        int n = recv(sockfd, temp, max_size, 0);
        if(n < 0) {
            ERROR("Unable to read from socket");
            exit(1);
        }
        if(n == 0) {
            ERROR("Client closed connection");
            break;
        }
        strcat(buf, temp);
        if(temp[n - 1] == delim) {
            break;
        }
        tot += n;
    }
    return tot;
}