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
#define PROMPT(msg, ...) printf("\033[1;32m"msg"\033[0m", ##__VA_ARGS__);
#define COMMAND_SIZE 1000
#define MAX_SIZE 10000

const char* code_200 = "200";
const char* code_500 = "500";
const char* code_600 = "600";

void sendFile(int sockfd, int fd) {
    struct stat st;
    fstat(fd, &st);
    long int file_size = st.st_size;

    // DEBUG("File size: %ld", file_size);

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
        short t = htons(n);
        memcpy(buf + 1, &t, 2);
        memcpy(buf + 3, file_buf, n);

        send(sockfd, buf, n + 3, 0);
        len += n;
    } while (len < file_size);
    // DEBUG("bahar aaya sendfile");
}

void recvFile(int sockfd, int fd) {
    char buf[MAX_SIZE];
    short rem = 0;
    int type = 1, len = 0;
    // int data = 0;
    char curr_type;
    char temp_len[2];
    int done = 0;
    do {
        int sz = (rem > 0 ? rem : 1);
        sz = (sz < MAX_SIZE ? sz : MAX_SIZE);
        int n = recv(sockfd, buf, sz, 0);
        // DEBUG("Received %d bytes", n);
        for (int i = 0; i < n; i++) {
            if (type) {
                curr_type = buf[i];
                type = 0; len = 1;
                // data = 0;
            } else if (len == 1) {
                temp_len[0] = buf[i];
                len = 2;
            } else if (len == 2) {
                temp_len[1] = buf[i];
                len = 0;
                // data = 1;
                // DEBUG("left: %d", (int)temp_len[0]);
                // DEBUG("right: %d", (int)temp_len[1]);
                rem = ntohs(*(short*)temp_len);
                // DEBUG("Remaining: %d", rem);
                if (!rem) {
                    done = 1;
                }
            } else {
                int can_write = ((rem < n - i) ? rem : n - i);
                write(fd, buf + i, can_write);
                // DEBUG("Wrote %d bytes", can_write);
                rem -= can_write;
                i += can_write - 1;
                // DEBUG("Can write: %d", can_write);
                // DEBUG("Remaining: %d", rem);
                // DEBUG("type: %c", curr_type);
                if (rem == 0) {
                    if (curr_type == 'L') {
                        done = 1;
                    }
                    type = 1;
                    // data = 0;
                }
            }
        }
    } while (!done);
}

int receive(int sockfd, char* buf, int SIZE, char delim) {
    memset(buf, 0, SIZE);
    char temp[1];
    int tot = 0;
    while(1) {
        memset(temp, 0, sizeof(temp));
        // DEBUG("idhar...");
        int n = recv(sockfd, temp, 1, 0);
        // DEBUG("udhar...");
        if(n < 0) {
            ERROR("Unable to read from socket");
            exit(1);
        }
        if(n == 0) {
            ERROR("Client closed connection");
            break;
        }
        strcat(buf, temp);
        tot += n;
        if(temp[n - 1] == delim) {
            break;
        }
    }
    // DEBUG("Buffer: %s", buf);
    return tot;
}

int connection_open = 0;
int authenticated = 0;
int args = 0;


char** parseCommand(char* command, int* num) {
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

    *num = cnt;

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
    // DEBUG("cmd[0]: %s, %d", cmd[0], (int)strlen(cmd[0]));
    // DEBUG("cmd[1]: %s, %d", cmd[1], (int)strlen(cmd[1]));
    // DEBUG("command: %s, %d", command, (int)strlen(command));
    return cmd;
}

int _open(char** cmd) {
    int PORT = atoi(cmd[2]);
    if (PORT < 20000 || PORT > 65535) {
        ERROR("Enter valid port no. (between 20000 and 65535)");
        return -1;
    }
    int sockfd;
    struct sockaddr_in serv_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERROR("Unable to create socket");
        return sockfd;
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;     // The family is AF_INET for the internet family
    serv_addr.sin_port = htons(PORT);
    int s = inet_aton(cmd[1], &serv_addr.sin_addr);
    if (s == 0) {
        ERROR("Not in presentation format");
        return -1;
    } else if (s < 0) {
        ERROR("Unable to convert IP address");
        return -1;
    }
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        ERROR("Unable to connect to server");
        return -1;
    }
    if (sockfd >= 0) {
        connection_open = 1;
        SUCCESS("Connection opened successfully.");
    } else {
        ERROR("Error in opening connection.");
    }
    return sockfd;
}

void _user(int sockfd, char* command) {
    int n = send(sockfd, command, strlen(command) + 1, 0);
    // DEBUG("Sent %d bytes", n);
    char code[5];
    n = receive(sockfd, code, 5, '\0');
    // DEBUG("Received %d bytes", n);
    if (!strcmp(code_200, code)) {
        SUCCESS("%s, Command executed successfully", code);
    } else {
        ERROR("%s, Error executing command", code);
    }
} 

void _pass(int sockfd, char* command) {
    send(sockfd, command, strlen(command) + 1, 0);
    char code[5];
    receive(sockfd, code, 5, '\0');
    if (!strcmp(code_200, code)) {
        SUCCESS("%s, Command executed successfully", code);
        authenticated = 1;
    } else {
        ERROR("%s, Error executing command", code);
    }
}

void _cd(int sockfd, char* command) {
    send(sockfd, command, strlen(command)+1, 0);
    char code[5];
    receive(sockfd, code, 5, '\0');
    if (!strcmp(code_200, code)) {
        SUCCESS("%s, Command executed successfully", code);
    } else {
        ERROR("%s, Error executing command", code);
    }
}

void _lcd(char** cmd) {
    if (chdir(cmd[1]) == 0) {
        SUCCESS("Directory changed");
    } else {
        ERROR("Directory could not changed");
    }
}

void _dir(int sockfd, char* command) {
    send(sockfd, command, strlen(command)+1, 0);
    char content[MAX_SIZE], temp[MAX_SIZE];
    memset(content, '\0', sizeof(content));
    int ind = 0, end = 0;
    char prev = ' ';
    while(!end) {
        memset(temp, 0, sizeof(temp));
        int n = recv(sockfd, temp, MAX_SIZE, 0);
        if(n < 0) {
            ERROR("Unable to read from socket\n");
            return;
        }
        for(int i = 0; i < n; i++) {
            content[ind++] = temp[i];
            if(temp[i] == '\0') {
                if(prev == '\0') {
                    end = 1;
                    break;
                }
                SUCCESS("%s", content);
                memset(content, 0, sizeof(content));
                ind = 0;
            }
            prev = temp[i];
        }
    }
}

int _get(int sockfd, char** cmd, char* command) {
    if(cmd[1][0] == '.') {
        ERROR("Remote filename cannot begin with dot");
        return -1;
    }
    if(cmd[2][0] == '.') {
        ERROR("Local filename cannot begin with dot");
        return -1;
    }

    int fd = open(cmd[2], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        ERROR("Unable to open file");
        return -1;
    }
    send(sockfd, command, strlen(command) + 1, 0);
    char code[5];
    receive(sockfd, code, 5, '\0');
    if (strcmp(code_200, code)) {
        ERROR("%s, Error executing command", code);
        return -1;
    }
    recvFile(sockfd, fd);
    close(fd);
    return 1;
}

int _put(int sockfd, char** cmd, char* command) {
    if(cmd[1][0] == '.') {
        ERROR("Local filename cannot begin with dot");
        return -1;
    }
    if(cmd[2][0] == '.') {
        ERROR("Remote filename cannot begin with dot");
        return -1;
    }

    int fd = open(cmd[1], O_RDONLY);
    if (fd < 0) {
        ERROR("Unable to open file");
        return -1;
    }
    send(sockfd, command, strlen(command) + 1, 0);
    char code[5];
    receive(sockfd, code, 5, '\0');
    if (strcmp(code_200, code)) {
        ERROR("%s, Error executing command", code);
        return -1;
    }
    sendFile(sockfd, fd);
    close(fd);
    return 1;
}

void _mget(int sockfd, char** cmd) {
    // DEBUG("%d", args);
    for (int i = 1; i < args; i++) {
        char temp_command[COMMAND_SIZE];
        memset(temp_command, '\0', sizeof(temp_command));
        sprintf(temp_command, "get %s %s", cmd[i], cmd[i]);
        // DEBUG("%s", temp_command);
        int n;
        char** temp_cmd = parseCommand(temp_command, &n);
        if (_get(sockfd, temp_cmd, temp_command) < 0) {
            // ERROR("Failed at index %d", i);
            break;
        }
    }
    return;
}

void _mput(int sockfd, char** cmd) {
    // DEBUG("%d", args);
    for (int i = 1; i < args; i++) {
        char temp_command[COMMAND_SIZE];
        memset(temp_command, '\0', sizeof(temp_command));
        sprintf(temp_command, "put %s %s", cmd[i], cmd[i]);
        // DEBUG("%s", temp_command);
        int n;
        char** temp_cmd = parseCommand(temp_command, &n);
        if (_put(sockfd, temp_cmd, temp_command) < 0) {
            // ERROR("Failed at index %d", i);
            break;
        }
    }
    return;
}

void _quit(int sockfd) {
    if (connection_open) {
        close(sockfd);
    }
    exit(0);
}

int main () {
    int sockfd;

    while (1) {
        char command[COMMAND_SIZE];
        memset(command, '\0', sizeof(command));

        PROMPT("myFTP> "); // later maybe add curr_dir
        
        fgets(command, COMMAND_SIZE, stdin);
        command[strlen(command) - 1] = '\0';

        char** cmd = parseCommand(command, &args);

        if (!cmd[0]) {
            continue;
        } else if (!connection_open && strcmp(cmd[0], "open")) {
            printf("First command has to be open\n");
            continue;
        }

        if(strcmp(cmd[0], "open") && strcmp(cmd[0], "user") && strcmp(cmd[0], "pass") && !authenticated) {
            ERROR("User not authenticated");
            continue;
        }

        if (!strcmp(cmd[0], "open")) {
            sockfd = _open(cmd);
        } else if (!strcmp(cmd[0], "user")) {
            _user(sockfd, command);
        } else if (!strcmp(cmd[0], "pass")) {
            _pass(sockfd, command);
        } else if (!strcmp(cmd[0], "cd")) {
            _cd(sockfd, command);
        } else if (!strcmp(cmd[0], "lcd")) {
            _lcd(cmd);
        } else if (!strcmp(cmd[0], "dir")) {
            _dir(sockfd, command);
        } else if (!strcmp(cmd[0], "get")) {
            _get(sockfd, cmd, command);
        } else if (!strcmp(cmd[0], "put")) {
            _put(sockfd, cmd, command);
        } else if (!strcmp(cmd[0], "mget")) {
            _mget(sockfd, cmd);
        } else if (!strcmp(cmd[0], "mput")) {
            _mput(sockfd, cmd);
        } else if (!strcmp(cmd[0], "quit")) {
            _quit(sockfd);
        } else {
            // ERROR("Unrecognized command");
        }
    }
    return 0;
}