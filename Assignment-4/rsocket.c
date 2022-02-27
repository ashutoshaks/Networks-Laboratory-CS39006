#include "rsocket.h"

#include <arpa/inet.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
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

#define ACK_TYPE 'A'
#define DATA_TYPE 'D'

#define TYPE_SIZE sizeof(char)
#define MSG_ID_SIZE sizeof(short)
#define MAX_MSG_SIZE 200
#define MAX_FRAME_SIZE (TYPE_SIZE + MSG_ID_SIZE + MAX_MSG_SIZE)

#define MAX_TBL_SIZE 100

/*
Frame Format:
1 byte - char - type - D for data, A for ack
2 bytes - short - msg_id
arbitary no. of bytes - msg
*/

short msg_cntr = 0;
pthread_t tid_R, tid_S;

typedef struct _recvd_table_entry {
    short msg_id;
    char *msg;
    size_t msg_len;
    struct sockaddr src_addr;
    socklen_t addrlen;
} recvd_table_entry;

typedef struct _recvd_table {
    recvd_table_entry *messages[MAX_TBL_SIZE];
    int in;
    int out;
    int count;
    pthread_mutex_t mutex;
} recvd_table;

recvd_table *recvd_msg_tbl;

void init_recvd_table() {
    recvd_msg_tbl = (recvd_table *)malloc(sizeof(recvd_table));
    for (int i = 0; i < MAX_TBL_SIZE; i++) {
        recvd_msg_tbl->messages[i] = NULL;
    }
    recvd_msg_tbl->in = 0;
    recvd_msg_tbl->out = 0;
    recvd_msg_tbl->count = 0;
    pthread_mutex_init(&recvd_msg_tbl->mutex, NULL);
}

void enqueue_recvd_table(char *buf, size_t len, struct sockaddr *src_addr,
                         socklen_t addrlen) {
    recvd_msg_tbl->messages[recvd_msg_tbl->in] = (recvd_table_entry *)malloc(sizeof(recvd_table_entry));
    recvd_table_entry *in_entry = recvd_msg_tbl->messages[recvd_msg_tbl->in];
    in_entry->msg_len = len;
    in_entry->src_addr = *src_addr;
    in_entry->addrlen = addrlen;
    if (len > 0) {
        in_entry->msg_id = ntohs(*(short *)(buf + TYPE_SIZE));
        in_entry->msg = (char *)malloc(len * sizeof(char));
        memcpy(in_entry->msg, buf + TYPE_SIZE + MSG_ID_SIZE, len);
    }

    recvd_msg_tbl->in = (recvd_msg_tbl->in + 1) % MAX_TBL_SIZE;
    recvd_msg_tbl->count++;
}

size_t dequeue_recvd_table(void *buf, size_t len, struct sockaddr *src_addr,
                           socklen_t *addrlen) {
    recvd_table_entry *out_entry = recvd_msg_tbl->messages[recvd_msg_tbl->out];
    size_t copy_len = len < out_entry->msg_len ? len : out_entry->msg_len;
    if (copy_len > 0) {
        memcpy(buf, out_entry->msg, copy_len);
    }
    *src_addr = out_entry->src_addr;
    *addrlen = out_entry->addrlen;

    if (copy_len > 0) {
        free(out_entry->msg);
    }
    free(out_entry);
    recvd_msg_tbl->messages[recvd_msg_tbl->out] = NULL;
    recvd_msg_tbl->out = (recvd_msg_tbl->out + 1) % MAX_TBL_SIZE;
    recvd_msg_tbl->count--;

    return copy_len;
}

void free_recvd_table() {
    for (int i = 0; i < MAX_TBL_SIZE; i++) {
        if (recvd_msg_tbl->messages[i] != NULL) {
            free(recvd_msg_tbl->messages[i]->msg);
            free(recvd_msg_tbl->messages[i]);
        }
    }
    free(recvd_msg_tbl);
}

typedef struct _unackd_table_entry {
    int msg_id;
    char *msg;
    size_t msg_len;
    int flags;
    struct sockaddr dest_addr;
    socklen_t addrlen;
    struct timeval sent_time;
} unackd_table_entry;

typedef struct _unackd_table {
    unackd_table_entry *messages[MAX_TBL_SIZE];
    pthread_mutex_t mutex;
} unackd_table;

unackd_table *unackd_msg_table;

void init_unackd_table() {
    unackd_msg_table = (unackd_table *)malloc(sizeof(unackd_table));
    for (int i = 0; i < MAX_TBL_SIZE; i++) {
        unackd_msg_table->messages[i] = NULL;
    }
    pthread_mutex_init(&unackd_msg_table->mutex, NULL);
}

void insert_unackd_table(char *buf, size_t len, int flags, const struct sockaddr *dest_addr,
                         socklen_t addrlen, struct timeval sent_time) {
    for (int i = 0; i < MAX_TBL_SIZE; i++) {
        if (unackd_msg_table->messages[i] == NULL) {
            unackd_msg_table->messages[i] = (unackd_table_entry *)malloc(sizeof(unackd_table_entry));
            unackd_table_entry *curr_entry = unackd_msg_table->messages[i];
            curr_entry->msg = (char *)malloc(len * sizeof(char));
            curr_entry->msg_len = len;
            curr_entry->flags = flags;
            curr_entry->dest_addr = *(struct sockaddr *)dest_addr;
            curr_entry->addrlen = addrlen;
            curr_entry->sent_time = sent_time;
            memcpy(curr_entry->msg, buf, len);
            return;
        }
    }
    ERROR("Unable to insert into unackd_msg_tbl");
}

void delete_unackd_table(short msg_id) {
    for (int i = 0; i < MAX_TBL_SIZE; i++) {
        if (unackd_msg_table->messages[i] != NULL && unackd_msg_table->messages[i]->msg_id == msg_id) {
            free(unackd_msg_table->messages[i]->msg);
            free(unackd_msg_table->messages[i]);
            unackd_msg_table->messages[i] = NULL;
            return;
        }
    }
    ERROR("Message with id %d not found in unackd_msg_table", msg_id);
}

void free_unackd_table() {
    for (int i = 0; i < MAX_TBL_SIZE; i++) {
        if (unackd_msg_table->messages[i] != NULL) {
            free(unackd_msg_table->messages[i]->msg);
            free(unackd_msg_table->messages[i]);
        }
    }
    free(unackd_msg_table);
}

/*
    function for thread R

    wait for message by calling recvfrom (this will get a frame)
    if data message, call dropMessage, accordingly add to received-message table and send ack
    for sending ack DON'T call r_sendto
    if ack message, delete from unacknowledged-message table

    char buf[MAX_FRAME_SIZE];
    ssize_t n = recv_from(sockfd, buf, MAX_FRAME_SIZE, 0, src_addr, addrlen);
    free(table_entry_ptr);
    table_entry_ptr = (char*)malloc(n * sizeof(char));
*/
void *recv_thread(void *arg) {
    int sockfd = *(int *)arg;
    while (1) {
        struct sockaddr src_addr;
        socklen_t addrlen = sizeof(src_addr);
        char buf[MAX_FRAME_SIZE];
        pthread_testcancel();  // cancellation point
        ssize_t len = recvfrom(sockfd, buf, MAX_FRAME_SIZE, 0, &src_addr, &addrlen);
        if (len >= 0) {
            int drop = ((len > 0) && dropMessage(P));
            if (drop) {
                continue;
            }
            if (len == 0 || (len > 0 && buf[0] == DATA_TYPE)) {
                pthread_mutex_lock(&recvd_msg_tbl->mutex);
                while (recvd_msg_tbl->count == MAX_TBL_SIZE) {
                    pthread_mutex_unlock(&recvd_msg_tbl->mutex);
                    usleep(100);
                    pthread_mutex_lock(&recvd_msg_tbl->mutex);
                }
                enqueue_recvd_table(buf, len, &src_addr, addrlen);
                pthread_mutex_unlock(&recvd_msg_tbl->mutex);

                // need to send acknowledgement if len > 0
                if (len > 0) {
                    char ack_frame[MAX_FRAME_SIZE];
                    ack_frame[0] = ACK_TYPE;
                    short msg_id = ntohs(*(short *)(buf + TYPE_SIZE));
                    short t = htons(msg_id);
                    memcpy(ack_frame + TYPE_SIZE, &t, MSG_ID_SIZE);
                    // *(short *)(ack_frame + TYPE_SIZE) = *(short *)(buf + TYPE_SIZE);
                    sendto(sockfd, ack_frame, TYPE_SIZE + MSG_ID_SIZE, 0, &src_addr, addrlen);
                }
            } else {
                assert(len > 0 && buf[0] == ACK_TYPE);
                short msg_id = *(short *)(buf + TYPE_SIZE);
                pthread_mutex_lock(&unackd_msg_table->mutex);
                delete_unackd_table(msg_id);
                pthread_mutex_unlock(&unackd_msg_table->mutex);
            }
        } else {
            perror("recvfrom");
        }
        pthread_testcancel();  // cancellation point
    }
}

/*
    function for thread S

    sleep for time T
    on waking up, check unacknowledged-message table
    if any message has timed out, resend it and update its timestamp (resend using sendto, not r_sendto)
    do this for all messages in the unacknowledged-message table
*/
void *retransmit_thread(void *arg) {
    int sockfd = *(int *)arg;
    while (1) {
        sleep(T);
        pthread_mutex_lock(&unackd_msg_table->mutex);
        for (int i = 0; i < MAX_TBL_SIZE; i++) {
            unackd_table_entry *curr_entry = unackd_msg_table->messages[i];
            if (curr_entry != NULL) {
                struct timeval curr_time, diff;
                gettimeofday(&curr_time, NULL);
                timersub(&curr_time, &curr_entry->sent_time, &diff);
                if (diff.tv_sec > TIMEOUT) {
                    // check return value to see if other side has closed connection
                    curr_entry->sent_time = curr_time;
                    unackd_table_entry send_entry = *curr_entry;
                    pthread_mutex_unlock(&unackd_msg_table->mutex);
                    sendto(sockfd, send_entry.msg, send_entry.msg_len, send_entry.flags, &send_entry.dest_addr, send_entry.addrlen);
                    pthread_mutex_lock(&unackd_msg_table->mutex);
                }
            }
        }
        pthread_mutex_unlock(&unackd_msg_table->mutex);
        pthread_testcancel();  // cancellation point
    }
}

int dropMessage(float p) {
    float rnd = (float)rand() / (float)RAND_MAX;
    return (rnd < p);
}

/*
    Open a UDP socket
    Create threads R and S
    Allocate space for the 2 tables
    Initialize all char ptrs to NULL
*/
int r_socket(int domain, int type, int protocol) {
    int sockfd = socket(domain, SOCK_DGRAM, protocol);
    if (sockfd >= 0) {
        init_recvd_table();
        init_unackd_table();
        pthread_create(&tid_R, NULL, recv_thread, &sockfd);
        pthread_create(&tid_S, NULL, retransmit_thread, &sockfd);
    }
    return sockfd;
}

/*
    bind
*/
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int ret = bind(sockfd, addr, addrlen);
    return ret;
}

/*
    send the message (by adding extra bytes to make the frame)
    add the message to unacknowledged-message table
*/
ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags,
                 const struct sockaddr *dest_addr, socklen_t addrlen) {
    char data_frame[MAX_FRAME_SIZE];
    data_frame[0] = DATA_TYPE;
    short msg_id = htons(msg_cntr++);
    memcpy(data_frame + TYPE_SIZE, &msg_id, MSG_ID_SIZE);
    memcpy(data_frame + TYPE_SIZE + MSG_ID_SIZE, buf, len);
    struct timeval sent_time;
    gettimeofday(&sent_time, NULL);
    ssize_t sent_len = sendto(sockfd, data_frame, TYPE_SIZE + MSG_ID_SIZE + len, flags, dest_addr, addrlen);

    if (sent_len > 0) {
        pthread_mutex_lock(&unackd_msg_table->mutex);
        insert_unackd_table(data_frame, len + TYPE_SIZE + MSG_ID_SIZE, flags, dest_addr, addrlen, sent_time);
        pthread_mutex_unlock(&unackd_msg_table->mutex);
    }
    return sent_len;
}

/*
    check received-message table, if there return that message and delete from the table
    if no message there, sleep for some time and check again
    so essentially, this is a blocking call until we get any message in the table
*/
ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,
                   struct sockaddr *src_addr, socklen_t *addrlen) {
    while (1) {
        pthread_mutex_lock(&recvd_msg_tbl->mutex);
        if (recvd_msg_tbl->count > 0) {
            size_t recv_len = dequeue_recvd_table(buf, len, src_addr, addrlen);
            pthread_mutex_unlock(&recvd_msg_tbl->mutex);
            return recv_len;
        } else {
            pthread_mutex_unlock(&recvd_msg_tbl->mutex);
            sleep(1);
        }
    }
}

/*
    kill threads R and S
    destroy mutexes
    free the tables (first free all char ptrs)
    close the socket
*/
int r_close(int fd) {
    pthread_cancel(tid_R);
    pthread_cancel(tid_S);
    pthread_join(tid_R, NULL);
    pthread_join(tid_S, NULL);

    pthread_mutex_destroy(&recvd_msg_tbl->mutex);
    pthread_mutex_destroy(&unackd_msg_table->mutex);

    free_recvd_table();
    free_unackd_table();

    int ret = close(fd);
    return ret;
}

int main() {
    printf("%ld\n", sizeof(recvd_msg_tbl));
    // struct timeval tv;
    // gettimeofday(&tv, NULL);
    // printf("%ld\n", tv.tv_sec);
    // printf("%ld\n", tv.tv_usec);
}