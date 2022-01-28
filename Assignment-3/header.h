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

#define ERROR(msg, ...) printf("\033[1;31m[ERROR] "msg" \033[0m\n", ##__VA_ARGS__);
#define SUCCESS(msg, ...) printf("\033[1;36m[INFO] "msg" \033[0m\n", ##__VA_ARGS__);
#define DEBUG(msg, ...) printf("\033[1;34m[DEBUG] "msg" \033[0m\n", ##__VA_ARGS__);
#define PROMPT(msg, ...) printf("\033[1;32m"msg" \033[0m", ##__VA_ARGS__);

const int COMMAND_SIZE = 1000;
const int MAX_SIZE = 1000;

const char* code_200 = "200";
const char* code_500 = "500";
const char* code_600 = "600";

int isAuthenticated() {

}

void sendFile(int sockfd, char *filename) {

}

void recvFile(int sockfd, char *filename) {

}