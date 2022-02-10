#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define ERROR(msg, ...) printf("\033[1;31m[ERROR] " msg " \033[0m\n", ##__VA_ARGS__);
#define SUCCESS(msg, ...) printf("\033[1;36m[SUCCESS] " msg " \033[0m\n", ##__VA_ARGS__);
#define INFO(msg, ...) printf("\033[1;34m[INFO] " msg " \033[0m\n", ##__VA_ARGS__);
#define PROMPT(msg, ...) printf("\033[1;32m" msg "\033[0m", ##__VA_ARGS__);

#define COMMAND_SIZE 1000
#define MAX_SIZE 10000

const char* code_200 = "200";
const char* code_500 = "500";
const char* code_600 = "600";

int connection_open = 0;
int authenticated = 0;
int args = 0;

// Helper function to send a file (with file descriptor fd) from a socket sockfd
void sendFile(int sockfd, int fd) {
    struct stat st;
    fstat(fd, &st);
    long int file_size = st.st_size;

    long int len = 0;
    int sz = MAX_SIZE - 3;
    char buf[MAX_SIZE];
    char file_buf[sz];
    do {
        short n = read(fd, file_buf, sz);
        if (len + n < file_size) {
            buf[0] = 'M';  // More blocks will follow
        } else {
            buf[0] = 'L';  // Last block
        }
        short t = htons(n);
        memcpy(buf + 1, &t, 2);        // Number of bytes in this block
        memcpy(buf + 3, file_buf, n);  // The content of this block
        send(sockfd, buf, n + 3, 0);
        len += n;
    } while (len < file_size);
}

// Helper function to receive a file (with file descriptor fd) to a socket sockfd
void recvFile(int sockfd, int fd) {
    char buf[MAX_SIZE];
    short rem = 0;
    int type = 1, len = 0;
    char curr_type;  // To denote 'M' or 'L'
    char temp_len[2];
    int done = 0;
    do {
        int sz = (rem > 0 ? rem : 1);
        sz = (sz < MAX_SIZE ? sz : MAX_SIZE);
        int n = recv(sockfd, buf, sz, 0);
        for (int i = 0; i < n; i++) {
            if (type) {  // Indicates present byte is type of the block
                curr_type = buf[i];
                type = 0;
                len = 1;
            } else if (len == 1) {  // First byte of the length of the block
                temp_len[0] = buf[i];
                len = 2;
            } else if (len == 2) {  // Second byte of the length of the block
                temp_len[1] = buf[i];
                len = 0;
                rem = ntohs(*(short*)temp_len);  // temp_len has the number of bytes in a block
                if (!rem) {
                    done = 1;
                }
            } else {  // Data of the block
                int can_write = ((rem < n - i) ? rem : n - i);
                write(fd, buf + i, can_write);
                rem -= can_write;
                i += can_write - 1;
                if (rem == 0) {
                    if (curr_type == 'L') {
                        done = 1;
                    }
                    type = 1;
                }
            }
        }
    } while (!done);
}

// Helper function to receive data using TCP until a delimiter is encountered
int receive(int sockfd, char* buf, int size, char delim) {
    memset(buf, 0, size);
    char temp[1];
    int tot = 0;
    while (1) {
        memset(temp, 0, sizeof(temp));
        int n = recv(sockfd, temp, 1, 0);
        if (n < 0) {
            ERROR("Unable to read from socket");
            exit(1);
        }
        if (n == 0) {
            ERROR("Client closed connection");
            break;
        }
        strcat(buf, temp);
        tot += n;
        if (temp[n - 1] == delim) {
            break;
        }
    }
    return tot;
}

// Function to parse the command and break it into appropriate tokens
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

    char** cmd = (char**)malloc(cnt * sizeof(char*));
    i = 0, cnt = 0;
    char buf[100];
    while (i < strlen(command)) {
        memset(buf, 0, sizeof(buf));
        int j = 0;
        while (i < strlen(command) && command[i] != ' ') {
            buf[j++] = command[i++];
        }
        cmd[cnt] = (char*)malloc((strlen(buf) + 1) * sizeof(char));
        strcpy(cmd[cnt++], buf);
        while (i < strlen(command) && command[i] == ' ') {
            i++;
        }
    }
    return cmd;
}

// Function to handle the 'open' command
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
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    int s = inet_aton(cmd[1], &serv_addr.sin_addr);
    if (s == 0) {
        ERROR("Not in presentation format");
        return -1;
    } else if (s < 0) {
        ERROR("Unable to convert IP address");
        return -1;
    }
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        ERROR("Unable to connect to server");
        return -1;
    }
    if (sockfd >= 0) {
        connection_open = 1;
        SUCCESS("Connection opened successfully");
    } else {
        ERROR("Error in opening connection");
    }
    return sockfd;
}

// Function to handle the 'user' command
void _user(int sockfd, char* command) {
    int n = send(sockfd, command, strlen(command) + 1, 0);
    char code[5];
    n = receive(sockfd, code, 5, '\0');
    if (!strcmp(code_200, code)) {
        SUCCESS("Command executed successfully");
    } else {
        ERROR("%s, Error executing command", code);
    }
}

// Function to handle the 'pass' command
void _pass(int sockfd, char* command) {
    send(sockfd, command, strlen(command) + 1, 0);
    char code[5];
    receive(sockfd, code, 5, '\0');
    if (!strcmp(code_200, code)) {
        SUCCESS("Command executed successfully");
        authenticated = 1;
    } else {
        ERROR("%s, Error executing command", code);
    }
}

// Function to handle the 'cd' command
void _cd(int sockfd, char* command) {
    send(sockfd, command, strlen(command) + 1, 0);
    char code[5];
    receive(sockfd, code, 5, '\0');
    if (!strcmp(code_200, code)) {
        SUCCESS("Command executed successfully");
    } else {
        ERROR("%s, Error executing command", code);
    }
}

// Function to handle the 'lcd' command
void _lcd(char** cmd) {
    if (chdir(cmd[1]) == 0) {
        SUCCESS("Directory changed");
    } else {
        ERROR("Directory could not be changed");
    }
}

// Function to handle the 'dir' command
void _dir(int sockfd, char* command) {
    send(sockfd, command, strlen(command) + 1, 0);
    char content[MAX_SIZE], temp[MAX_SIZE];
    memset(content, 0, sizeof(content));
    int ind = 0, end = 0;
    char prev = '\0';
    while (!end) {
        memset(temp, 0, sizeof(temp));
        int n = recv(sockfd, temp, MAX_SIZE, 0);
        if (n < 0) {
            ERROR("Unable to read from socket\n");
            return;
        }
        for (int i = 0; i < n; i++) {
            content[ind++] = temp[i];
            if (temp[i] == '\0') {
                if (prev == '\0') {
                    end = 1;
                    break;
                }
                printf("%s\n", content);
                memset(content, 0, sizeof(content));
                ind = 0;
            }
            prev = temp[i];
        }
    }
}

// Function to handle the 'get' command
int _get(int sockfd, char** cmd, char* command) {
    if (cmd[1][0] == '.') {
        ERROR("Remote filename cannot begin with dot");
        return -1;
    }
    if (cmd[2][0] == '.') {
        ERROR("Local filename cannot begin with dot");
        return -1;
    }

    int fd = open(cmd[2], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR);
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
    SUCCESS("Command executed successfully");
    return 1;
}

// Function to handle the 'put' command
int _put(int sockfd, char** cmd, char* command) {
    if (cmd[1][0] == '.') {
        ERROR("Local filename cannot begin with dot");
        return -1;
    }
    if (cmd[2][0] == '.') {
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
    SUCCESS("Command executed successfully");
    return 1;
}

// Function to handle the 'mget' command
void _mget(int sockfd, char** cmd) {
    for (int i = 1; i < args; i++) {
        char temp_command[COMMAND_SIZE];
        memset(temp_command, 0, sizeof(temp_command));
        sprintf(temp_command, "get %s %s", cmd[i], cmd[i]);
        int n;
        char** temp_cmd = parseCommand(temp_command, &n);
        if (_get(sockfd, temp_cmd, temp_command) < 0) {
            ERROR("Failed at file %s", cmd[i]);
            return;
        }
    }
}

// Function to handle the 'mput' command
void _mput(int sockfd, char** cmd) {
    for (int i = 1; i < args; i++) {
        char temp_command[COMMAND_SIZE];
        memset(temp_command, 0, sizeof(temp_command));
        sprintf(temp_command, "put %s %s", cmd[i], cmd[i]);
        int n;
        char** temp_cmd = parseCommand(temp_command, &n);
        if (_put(sockfd, temp_cmd, temp_command) < 0) {
            ERROR("Failed at file %s", cmd[i]);
            return;
        }
    }
}

// Function to handle the 'quit' command
void _quit(int sockfd) {
    if (connection_open) {
        close(sockfd);
    }
    exit(0);
}

int main() {
    int sockfd;
    while (1) {
        char command[COMMAND_SIZE];
        memset(command, 0, sizeof(command));
        PROMPT("myFTP> ");
        fgets(command, COMMAND_SIZE, stdin);
        command[strlen(command) - 1] = '\0';
        char** cmd = parseCommand(command, &args);

        if (!cmd[0]) {
            continue;
        } else if (!connection_open && strcmp(cmd[0], "open")) {
            ERROR("First command has to be open");
            continue;
        }
        if (strcmp(cmd[0], "open") && strcmp(cmd[0], "user") && strcmp(cmd[0], "pass") && !authenticated) {
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
            ERROR("Unrecognized command");
        }
    }
    return 0;
}