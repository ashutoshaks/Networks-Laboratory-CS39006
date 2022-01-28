#include "../header.h"

#define PORT 20000

char* usernames[100];
char* passwords[100];
int numUsers;

// functions
// _user
// _pass
// _cd
// _dir
// _get
// _put

void _user(int sockfd, char** cmd, int* uind) {
    for (int i = 0; i < numUsers; i++) {
        if (!strcmp(cmd[1], usernames[i])) {
            *uind = i;
            send(sockfd, code_200, strlen(code_200) + 1, 0);
            DEBUG("Matched at user %d", i);
            return;
        }
    }
    DEBUG("User not found");
    send(sockfd, code_500, strlen(code_500) + 1, 0);
}


void _pass(int sockfd, char** cmd, int* uind, int* authenticated) {
    if (!strcmp(cmd[1], passwords[*uind])) {
        DEBUG("Password matched");
        *authenticated = 1;
        send(sockfd, code_200, strlen(code_200) + 1, 0);
        return;
    }
    DEBUG("Password did not match");
    send(sockfd, code_500, strlen(code_500) + 1, 0);
}


void _cd(int sockfd, char** cmd) {
    if (chdir(cmd[1]) == 0) {
        char s[100];
        DEBUG("Directory changed: %s", getcwd(s, 100));
        send(sockfd, code_200, strlen(code_200) + 1, 0);
    } else {
        ERROR("Directory could not not changed");
        send(sockfd, code_500, strlen(code_500) + 1, 0);
    }
}


void _dir(int sockfd, char** cmd) {
    char s[100];
    DEBUG("Current directory: %s", getcwd(s, 100));
    DIR* dir = opendir(".");
    if (dir) {
        DEBUG("Starting directory listing");
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            send(sockfd, entry->d_name, strlen(entry->d_name) + 1, 0);
        }
        send(sockfd, "\0", 1, 0);
        closedir(dir);
    } else {
        ERROR("Directory could not be opened");
    }
}

void _get(int sockfd, char** cmd) {
    if(cmd[1][0] == '.') {
        ERROR("Filename cannot begin with dot");
        send(sockfd, code_500, strlen(code_500) + 1, 0);
        return;
    }

    int fd = open(cmd[1], O_RDONLY);
    if (fd < 0) {
        ERROR("File could not be opened");
        send(sockfd, code_500, strlen(code_500) + 1, 0);
        return;
    }
    send(sockfd, code_200, strlen(code_200) + 1, 0);
    sendFile(sockfd, fd);
    close(fd);
}


void _put(int sockfd, char** cmd) {
    if(cmd[2][0] == '.') {
        ERROR("Filename cannot begin with dot");
        send(sockfd, code_500, strlen(code_500) + 1, 0);
        return;
    }

    int fd = open(cmd[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        ERROR("File could not be opened");
        send(sockfd, code_500, strlen(code_500) + 1, 0);
        return;
    }
    send(sockfd, code_200, strlen(code_200) + 1, 0);
    recvFile(sockfd, fd);
    close(fd);
}

void parseUsers() {
    FILE* fp = fopen("user.txt", "r");
    if (fp == NULL) {
        ERROR("Unable to open user.txt");
        exit(1);
    }
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    numUsers = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        if(line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        char* user = strtok(line, " ");
        char* pass = strtok(NULL, " ");
        usernames[numUsers] = (char*) malloc((strlen(user) + 1) * sizeof(char));
        passwords[numUsers] = (char*) malloc((strlen(pass) + 1) * sizeof(char));
        strcpy(usernames[numUsers], user);
        strcpy(passwords[numUsers], pass);
        numUsers++;
    }
    fclose(fp);
}


char** parseCommand(char* command) {
    for (int i = 0; i < strlen(command); i++) {
        if (command[i] == ',') {
            command[i] = ' ';
        }
    }
    int cnt = 0, i = 0;
    while (i < strlen(command)) {
        while (i < strlen(command) && command[i] != ' ') {
            i++;
        }   
        cnt++;
        while (i < strlen(command) && command[i] == ' ') {
            i++;
        } 
    }

    char** cmd = (char**) malloc(cnt * sizeof(char*));
    i = 0, cnt = 0;
    char buf[100];
    while (i < strlen(command)) {
        memset(buf, '\0', sizeof(buf));
        int j = 0;
        while (i < strlen(command) && command[i] != ' ') {
            buf[j++] = command[i++];
        }  
        cmd[cnt] = (char*) malloc((strlen(buf) + 1) * sizeof(char));
        strcpy(cmd[cnt++], buf);
        while (i < strlen(command) && command[i] == ' ') {
            i++;
        } 
    }
    return cmd;
}


int main () {
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    int clilen = sizeof(cli_addr);
    int opt = 1;

    parseUsers();

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERROR("Cannot create socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        ERROR("Unable to bind local address");
        exit(1);
    }

    listen(sockfd, 5);

    while (1) {
        if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen)) < 0) {
            ERROR("Unable to accept connection");
            exit(1);
        }

        if (fork() == 0) {
            close(sockfd);
            int cnt = 0;

            int uind = -1;
            int authenticated = 0;

            DEBUG("Child process started");

            while (1) {
                // receive a command from client
                char command[COMMAND_SIZE];
                int n = receive(newsockfd, command, COMMAND_SIZE, '\0');
                DEBUG("Command received: %s", command);
                if (n == 0) {
                    DEBUG("Client disconnected");
                    break;
                }
                
                char** cmd = parseCommand(command);

                if (!cmd[0]) {
                    continue;
                }

                if (!authenticated && uind == -1 && strcmp(cmd[0], "user")) {
                    ERROR("First command sent to the server has to be user");
                    send(newsockfd, code_600, strlen(code_600) + 1, 0);
                    continue;
                }
                if (!authenticated && uind != -1 && strcmp(cmd[0], "pass")) {
                    ERROR("Second command sent to the server has to be pass");
                    uind = -1;
                    send(newsockfd, code_600, strlen(code_600) + 1, 0);
                    continue;
                }

                if(strcmp(cmd[0], "user") && strcmp(cmd[0], "pass") && !authenticated) {
                    ERROR("User not authenticated");
                    continue;
                }

                if (!strcmp(cmd[0], "user")) {
                    _user(newsockfd, cmd, &uind);
                } else if (!strcmp(cmd[0], "pass")) {
                    _pass(newsockfd, cmd, &uind, &authenticated);
                } else if (!strcmp(cmd[0], "cd")) {
                    _cd(newsockfd, cmd);
                } else if (!strcmp(cmd[0], "dir")) {
                    _dir(newsockfd, cmd);
                } else if (!strcmp(cmd[0], "get")) {
                    _get(newsockfd, cmd);
                } else if (!strcmp(cmd[0], "put")) {
                    _put(newsockfd, cmd);
                } else {
                    ERROR("Unrecognized command");
                }

                cnt++;
            }

            close(newsockfd);
            exit(0);
        }

        close(newsockfd);
    }

    close(sockfd);
    return 0;
}