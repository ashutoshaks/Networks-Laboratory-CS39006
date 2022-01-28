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

char* usernames[100];
char* passwords[100];
int numUsers;


void parseUsers() {
    FILE* fp = fopen("server/user.txt", "r");
    if (fp == NULL) {
        // ERROR("Unable to open user.txt");
        exit(1);
    }
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    // memset(usernames, 0, sizeof(usernames));
    // memset(passwords, 0, sizeof(passwords));

    numUsers = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        if(line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        // printf("%s", line);
        char* user = strtok(line, " ");
        char* pass = strtok(NULL, " ");
        // printf("%s %s %d\n", user, pass, numUsers);
        usernames[numUsers] = (char*) malloc((strlen(user) + 1) * sizeof(char));
        passwords[numUsers] = (char*) malloc((strlen(pass) + 1) * sizeof(char));
        strcpy(usernames[numUsers], user);
        strcpy(passwords[numUsers], pass);
        usernames[numUsers] = user;
        passwords[numUsers] = pass;
        printf("%s %s %d\n", usernames[numUsers], passwords[numUsers], numUsers);
        numUsers++;
    }
    printf("%s %s\n", usernames[0], passwords[0]);
    printf("%s %s\n", usernames[1], passwords[1]);

    // for(int i = 0; i < numUsers; i++) {
    //     printf("%s %s\n", usernames[i], passwords[i]);
    // }
    fclose(fp);
}

int main()
{
    parseUsers();
}


