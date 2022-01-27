#include <stdio.h>
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

#include <netdb.h> // defines the hostent structure 
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define WHITE "\033[1;37m" 
#define RESET "\033[0m" 

const char* OPEN = "open";
const char* USER = "user";
const char* PASS =  "pass";
const char* CD = "cd";
const char* LCD = "lcd";
const char* DIR = "dir";
const char* GET = "get";
const char* PUT = "put";
const char* MGET = "mget";
const char* MPUT = "mput";
const char* QUIT = "quit";

int main () {

}