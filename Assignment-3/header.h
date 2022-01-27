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

#define _COLOR_CYAN "1;36"
#define _COLOR_RED "1;31"
#define _COLOR_BLUE "1;34"
#define _COLOR_GREEN "0;32"

FILE* _logFp = NULL;
void initLogger(const char* logFile) {
    _logFp = logFile ? fopen(logFile, "w") : stdout;
}

void log_print(FILE* fp, const char* fmt, ...) {
    if (_logFp != NULL)
        fp = _logFp;
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    fflush(fp);
    va_end(args);
}

#define __LOG_COLOR(FD, CLR, CTX, TXT, args...) log_print(FD, "\033[%sm[%s] \033[0m" TXT, CLR, CTX, ##args)
#define SUCCESS(TXT, args...) __LOG_COLOR(stdout, _COLOR_CYAN, "INFO", TXT, ##args)
#define ERROR(TXT, args...) __LOG_COLOR(stderr, _COLOR_RED, "ERROR", TXT, ##args)
#define DEBUG(TXT, args...) __LOG_COLOR(stdout, _COLOR_BLUE, "DEBUG", TXT, ##args)

const int COMMAND_SIZE = 1000;
const int MAX_SIZE = 1000;

int isAuthenticated() {

}

void sendFile(int sockfd, char *filename) {

}

void recvFile(int sockfd, char *filename) {

}