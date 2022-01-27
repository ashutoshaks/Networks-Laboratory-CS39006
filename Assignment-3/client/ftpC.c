#include "../header.h"

int connection_open = 0;
int authenticated = 0;

// char curr_dir[MAX_SIZE];

// if not string like 3224df so arg valid?
// int stoi (char *num) {
//     printf("%d\n", atoi(num));
//     return atoi(num);
//     int val = 0, i = 0;
//     while (i < strlen(num) && num[i] <= '9' && num[i] >= '0')   {
//         val *= 10;
//         val += (num[i++] - '0');
//     }
//     if (i != strlen(num)) { 
//         printf("Invalid number\n");
//         return 0;
//     }
//     return val;
// }

int _open (const char* command) {
    char* ipAddress = strtok(NULL, " ");
    if (!ipAddress) {
        printf("IP address not entered\n");
        return -1;
    }
    char* port = strtok(NULL, " ");
    if (!port) {
        printf("Port no. not entered\n");
        return -1;
    }
    int PORT = atoi(port);
    if (PORT < 20000 || PORT > 65535) {
        printf("Enter valid port no. (between 20000 and 65535)\n");
        return -1;
    }
    int sockfd;
    struct sockaddr_in serv_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        return sockfd;
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;     // The family is AF_INET for the internet family
    serv_addr.sin_port = htons(PORT);
    int s = inet_aton(ipAddress, &serv_addr.sin_addr);
    if (s == 0) {
        printf("Not in presentation format\n");
        return -1;
    } else if (s < 0) {
        perror("Unable to convert IP address\n");
        return -1;
    }
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Unable to connect to server\n");
        return -1;
    }
    if (sockfd >= 0) {
        connection_open = 1;
        printf("Connection opened successfully.\n");
    } else {
        printf("Error in opening connection.\n");
    }
    return sockfd;
}

void _user (const char* command, int sockfd) {
    return;
} 

void _pass (const char* command, int sockfd) {
    return;
}

void _cd (const char* command, int sockfd) {
    return;
}

void _lcd (const char* command) {
    if (authenticated) {
        char* dir_name = strtok(NULL, " ");
        if (!dir_name) {
            chdir("/home/vanshita");
        } else if (chdir(dir_name)) {
            printf("No such file or directory\n");
        }
    } else {
        printf("User not authenticated.\n");
    }
    return;
}

void _dir (const char* command, int sockfd) {
    return;
}

void _get (const char* command, int sockfd) {
    // if (!strcmp(command_name, GET)) {
    //  char* remote_file = strtok(NULL, " ");
    //  if (!remote_file) {
    //      printf(RED "Enter remote_file name.\n");
    //      continue;
    //  }
    //  char* local_file = strtok(NULL, " ");
    //  if (!local_file) {
    //      printf(RED "Enter local_file name.\n");
    //      continue;
    //  }
    //  int fd = open(local_file, O_CREAT | O_EXCL | O_RDWR);
    //  if (fd < 0) {
    //      printf(RED, "Unable to open file.\n");
    //      continue;
    //  }
    //  send(sockfd, command, strlen(command) + 1, 0);
    //  // processResponseGet(sockfd, local_file);
    // } 
    return;
}

void _put (const char* command, int sockfd) {
    // else if (!strcmp(command_name, PUT)) {
    //  char* local_file = strtok(NULL, " ");
    //  if (!local_file) {
    //      printf(RED "Enter local_file name.\n");
    //      continue;
    //  }
    //  char* remote_file = strtok(NULL, " ");
    //  if (!remote_file) {
    //      printf(RED "Enter remote_file name.\n");
    //      continue;
    //  }
    //  int fd = open(local_file, O_RDONLY);
    //  if (fd < 0) {
    //      printf(RED, "Unable to open the local file.\n");
    //      continue;
    //  }
    //  send(sockfd, command, strlen(command) + 1, 0);
    //  // processResponsePut(sockfd, local_file);
    // }
    return;
}

// stop on error
void _mget (const char* command, int sockfd) {
    return;
}

void _mput (const char* command, int sockfd) {
    return;
}

void _quit (int sockfd) {
    if (connection_open) {
        close(sockfd);
    }
    exit(0);
}

int main () {
    // memset(curr_dir, '\0', sizeof(curr_dir));
    char command[COMMAND_SIZE];
    int sockfd;

    while (1) {
        memset(command, '\0', sizeof(command));

        printf("myFTP> "); // later maybe add curr_dir
        
        fgets(command, COMMAND_SIZE, stdin);
        command[strlen(command) - 1] = '\0';

        char* command_name = strtok(command, " ");

        if (!command_name) {
            continue;
        }
        if (!connection_open && strcmp(command_name, "open")) {
            printf("First command has to be open\n");
            continue;
        }

        if (!strcmp(command_name, "open")) {
            sockfd = _open(command);
        } else if (!strcmp(command_name, "user")) {
            _user(command, sockfd);
        } else if (!strcmp(command_name, "pass")) {
            _pass(command, sockfd);
        } else if (!strcmp(command_name, "cd")) {
            _cd(command, sockfd);
        } else if (!strcmp(command_name, "lcd")) {
            _lcd(command);
        } else if (!strcmp(command_name, "dir")) {
            _dir(command, sockfd);
        } else if (!strcmp(command_name, "get")) {
            _get(command, sockfd);
        } else if (!strcmp(command_name, "put")) {
            _put(command, sockfd);
        } else if (!strcmp(command_name, "mget")) {
            _mget(command, sockfd);
        } else if (!strcmp(command_name, "mput")) {
            _mput(command, sockfd);
        } else if (!strcmp(command_name, "quit")) {
            _quit(sockfd);
        } else {
            // Unrecognised commad - should I send to the server or print unrecognised from the client side?
        }
    }
    return 0;
}

// 200 - execution success
// 500 - execution error general
// 600 - command order


// -get 
// permission issues or path issues, if a dir not exist do i create the dir or error
// spcae?

//put
// space?

//mget
// - comma, space?

// dir dhould list or list if it's a file or directory


// how to connect? will client know port number or it will try to connect to anyone?
// Should we check for the number of args?

// Error names descriptive or just error happened?

/*
The commands from 4 to 10 below should only be executed if the username and
password have already matched. Both the client and the server should check that
before executing any of these commands.
*/

// The file name cannot be relative to the current directory or parent directory (i.e., cannot start with . or ..).
// checks which side like this or an invalid command