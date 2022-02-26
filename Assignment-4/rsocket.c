#include "rsocket.h"

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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define TYPE_SIZE sizeof(char)
#define MSG_ID_SIZE sizeof(short)
#define MAX_MSG_SIZE 1000
#define MAX_FRAME_SIZE (TYPE_SIZE + MSG_ID_SIZE + MAX_MSG_SIZE)

#define MAX_TBL_SIZE 100

/*
Frame Format:
1 byte - char - type - D for data, A for ack
2 bytes - short - msg_id
arbitary no. of bytes - msg
*/

typedef struct _recvd_table_entry {
    short msg_id;
    char *msg;
    size_t msg_len;
    struct sockaddr *src_addr;
    socklen_t *addrlen;
} recvd_table_entry;

typedef struct _recvd_table {
    recvd_table_entry messages[MAX_TBL_SIZE];
    int in;
    int out;
    int count;
    // add mutex, semaphores
} recvd_table;

recvd_table *recvd_msg_tbl;

typedef struct _unackd_table_entry {
    int msg_id;
    char *msg;
    size_t msg_len;
    // int flags;
    struct sockaddr *dest_addr;
    socklen_t addrlen;
    struct timeval sent_time;
    short is_empty;
} unackd_table_entry;

typedef struct _unackd_table {
    unackd_table_entry messages[MAX_TBL_SIZE];
    // add mutex, semaphores
} unackd_table;

unackd_table *unackd_msg_table;

int r_socket(int domain, int type, int protocol) {
    /*
    Open a UDP socket
    Create threads R and S
    Allocate space for the 2 tables
    */
}

int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    /*
    bind
    */
}

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags,
                 const struct sockaddr *dest_addr, socklen_t addrlen) {
    /*
    send the message (by adding extra bytes to make the frame)
    add the message to unacknowledged-message table
    */
}

ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,
                   struct sockaddr *src_addr, socklen_t *addrlen) {
    /*
    check received-message table, if there return that message and delete from the table
    if no message there, sleep for some time and check again
    so essentially, this is a blocking call until we get any message in the table
    */
}

int r_close(int fd) {
    /*
    close the socket
    kill threads R and S
    free the tables
    */
}

/*
function for thread R

wait for message by calling recvfrom (this will get a frame)
if data message, call dropMessage, accordingly add to received-message table and send ack
for sending ack DON'T call r_sendto
if ack message, delete from unacknowledged-message table
*/

/*
function for thread S

sleep for time T
on waking up, check unacknowledged-message table
if any message has timed out, resend it and update its timestamp (resend using sendto, not r_sendto)
do this for all messages in the unacknowledged-message table
*/

int main() {
    printf("%ld\n", sizeof(recvd_msg_tbl));
    // struct timeval tv;
    // gettimeofday(&tv, NULL);
    // printf("%ld\n", tv.tv_sec);
    // printf("%ld\n", tv.tv_usec);
}