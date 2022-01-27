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
#include <netdb.h>
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define WHITE "\033[1;37m" 
#define RESET "\033[0m" 

const int COMMAND_SIZE = 1000;
// const int MAX_SIZE = 1000;

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

// char curr_dir[MAX_SIZE];

// if not string like 3224df so arg valid?
int stoi (char *num) {
    int val = 0, i = 0;
    while (i < strlen(num) && num[i] <= '9' && num[i] >= '0')	{
    	val *= 10;
    	val += (num[i++] - '0');
    }
    if (i == 0) { 
    	printf("Invalid number\n");
    	return 0;
    }
    return val;
}

int _open (const char* command, int &PORT, int& connection_open) {
	char* ipAddress = strtok(NULL, " ");
	if (!ipAddress) {
		printf(RED "IP address not entered\n");
		return -1;
	}
	char* port = strtok(NULL, " ");
	if (!port) {
		printf(RED "Port no. not entered\n");
		return -1;
	}
	PORT = stoi(port);
	if (PORT < 20000 || PORT > 65535) {
		printf(RED "Enter valid port no. (between 20000 and 65535)\n");
		return -1;
	}
	int sockfd = -1;
	struct sockaddr_in serv_addr;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror(RED "Unable to create socket\n");
        return sockfd;
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;     // The family is AF_INET for the internet family
    serv_addr.sin_port = htons(PORT);
    int s = inet_aton(ipAddress, &serv_addr.sin_addr);
    if (s == 0) {
        printf(RED "Not in presentation format\n");
        return -1;
    } else if (s < 0) {
        perror(RED "Unable to convert IP address\n");
        return -1;
    }
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror(RED "Unable to connect to server\n");
        return -1;
    }
    if (sockfd >= 0) {
    	connection_open = 1;
		printf(YELLOW "Connection opened successfully.\n");
		connection_open = 1;
	} else {
		printf(RED "Error in opening connection.\n");
	}
	return sockfd;
}

void _user (const char* command, int sockfd) {
	return;
} 

void _pass (const char* command, int sockfd, int& authenticated) {
	return;
}

void _cd (const char* command, int sockfd, const int authenticated) {
	return;
}

void _lcd (const char* command, const int authenticated) {
	if (authenticated) {
		char* dir_name = strtok(NULL, " ");
		if (!dir_name) {
			chdir("/home/vanshita");
		} else if (chdir(dir_name)) {
			printf("No such file or directory\n");
		}
	} else {
		printf(RED "User not authenticated.\n");
	}
	return;
}

void _dir (const char* command, int sockfd, const int authenticated) {
	return;
}

void _get (const char* command, int sockfd, const int authenticated) {
	// if (!strcmp(command_name, GET)) {
	// 	char* remote_file = strtok(NULL, " ");
	// 	if (!remote_file) {
	// 		printf(RED "Enter remote_file name.\n");
	// 		continue;
	// 	}
	// 	char* local_file = strtok(NULL, " ");
	// 	if (!local_file) {
	// 		printf(RED "Enter local_file name.\n");
	// 		continue;
	// 	}
	// 	int fd = open(local_file, O_CREAT | O_EXCL | O_RDWR);
	// 	if (fd < 0) {
	// 		printf(RED, "Unable to open file.\n");
	// 		continue;
	// 	}
	// 	send(sockfd, command, strlen(command) + 1, 0);
	// 	// processResponseGet(sockfd, local_file);
	// } 
	return;
}

void _put (const char* command, int sockfd, const int authenticated) {
	// else if (!strcmp(command_name, PUT)) {
	// 	char* local_file = strtok(NULL, " ");
	// 	if (!local_file) {
	// 		printf(RED "Enter local_file name.\n");
	// 		continue;
	// 	}
	// 	char* remote_file = strtok(NULL, " ");
	// 	if (!remote_file) {
	// 		printf(RED "Enter remote_file name.\n");
	// 		continue;
	// 	}
	// 	int fd = open(local_file, O_RDONLY);
	// 	if (fd < 0) {
	// 		printf(RED, "Unable to open the local file.\n");
	// 		continue;
	// 	}
	// 	send(sockfd, command, strlen(command) + 1, 0);
	// 	// processResponsePut(sockfd, local_file);
	// }
	return;
}

// stop on error
void _mget (const char* command, int sockfd, const int authenticated) {
	return;
}

void _mput (const char* command, int sockfd, const int authenticated) {
	return;
}

void _quit (int sockfd, const int connection_open) {
	if (connection_open) {
		close(sockfd);
	}
	exit(0);
}

int main () {
	// memset(curr_dir, '\0', sizeof(curr_dir));
	short int error_occ;
	char command[COMMAND_SIZE];
	int PORT, sockfd, connection_open = 0, authenticated = 0;

	while (1) {
		memset(command, '\0', sizeof(command));

		printf(GREEN "myFTP> " WHITE); // later maybe add curr_dir
		
		fgets(command, COMMAND_SIZE, stdin);
		command[strlen(command)-1] = '\0';

		char* command_name = strtok(command, " ");

		if (!command_name) {
			continue;
		} else if (!connection_open && strcmp(command_name, OPEN)) {
			printf(RED "First command has to be open\n");
			continue;
		}

		if (!strcmp(command_name, OPEN)) {
			sockfd = _open(command, PORT, connection_open);
		} else if (!strcmp(command_name, USER)) {
			_user(command, sockfd);
		} else if (!strcmp(command_name, PASS)) {
			_pass(command, sockfd, authenticated);
		} else if (!strcmp(command_name, CD)) {
			_cd(command, sockfd, authenticated);
		} else if (!strcmp(command_name, LCD)) {
			_lcd(command, authenticated);
		} else if (!strcmp(command_name, DIR)) {
			_dir(command, sockfd, authenticated);
		} else if (!strcmp(command_name, GET)) {
			_get(command, sockfd, authenticated);
		} else if (!strcmp(command_name, PUT)) {
			_put(command, sockfd, authenticated);
		} else if (!strcmp(command_name, MGET)) {
			_mget(command, sockfd, authenticated);
		} else if (!strcmp(command_name, MPUT)) {
			_mput(command, sockfd, authenticated);
		} else if (!strcmp(command_name, QUIT)) {
			_quit(sockfd, connection_open);
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


// Should we check for the number of args?