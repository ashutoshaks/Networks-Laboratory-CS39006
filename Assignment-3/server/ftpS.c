#include "../header.h"

int uname_entered = 0;
int authenticated = 0;

// functions
// _user
// _pass
// _cd
// _dir
// _get
// _put


// Concurrent Server
int main () {
    // intialize the server (socket)

    while(1) {
        // newsockfd = accept()
        if(fork() == 0) {
            // while for authenticate

            // while for commands
            while(1) {

            }
        }
    }
}

// send, recv

// void recieve(int sockfd, char *buffer, int size, char delim) {

// }

// recieve(sockfd, buff, MAXN, '\0');