#include <stdio.h>
#include <dirent.h>
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

void func (char* str) {
    str[0] = 'a'; str[1] = 'b';
    // printf("%s\n", str);
}

int main()
{
    char str[5];
    scanf("%s", str);
    func(str);
    printf("%s\n", str);
    return 0;
}
