#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <unistd.h>

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define WHITE "\033[1;37m" 
#define RESET "\033[0m"

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
// void initLogger(const char*);
// void log_print(FILE*, const char*, ...);

#define __LOG_COLOR(FD, CLR, CTX, TXT, args...) log_print(FD, "\033[%sm[%s] \033[0m" TXT, CLR, CTX, ##args)
// #define SUCCESS(TXT, args...) __LOG_COLOR(stdout, _COLOR_CYAN, "info", TXT, ##args)
// #define ERROR(TXT, args...) __LOG_COLOR(stderr, _COLOR_RED, "error", TXT, ##args)
// #define PROMPT(TXT, args...) __LOG_COLOR(stdout, _COLOR_GREEN, "", TXT, ##args)
// #define DEBUG(TXT, args...) __LOG_COLOR(stdout, _COLOR_BLUE, "debug", TXT, ##args)


#define SUCCESS(str) printf(CYAN str RESET);
#define ERROR(str) printf(RED str RESET);   // use this for status code error
#define PROMPT(str) printf(GREEN str RESET);
#define DEBUG(str) printf(BLUE str RESET);
#define DEBUG(str1, str2) printf(BLUE str1, str2 RESET);


int main()
{
    // char s[100];

    // // printing current working directory
    // printf("%s\n", getcwd(s, 100));

    // // using the command
    // int changed = chdir("/home/vanshita/Desktop");
    // printf("chenged = %d\n", changed);
    // int fd = open("P.txt", O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IXUSR);
    // printf("%d\n", fd);
    // // printing current working directory
    // printf("%s\n", getcwd(s, 100));
    // close(fd);
    // // after chdir is executed
    // return 0;

    DEBUG("%s", "hello\n");
}


