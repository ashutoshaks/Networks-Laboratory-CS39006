#ifndef __RSOCKET_H
#define __RSOCKET_H

#include <sys/types.h>
#include <sys/socket.h>

#define ERROR(msg, ...) printf("\033[1;31m[ERROR] " msg " \033[0m\n", ##__VA_ARGS__);
#define SUCCESS(msg, ...) printf("\033[1;36m[SUCCESS] " msg " \033[0m\n", ##__VA_ARGS__);
#define INFO(msg, ...) printf("\033[1;34m[INFO] " msg " \033[0m\n", ##__VA_ARGS__);
#define PROMPT(msg, ...) printf("\033[1;32m" msg "\033[0m", ##__VA_ARGS__);

#define SOCK_MRP 15
#define T 2
#define P 0.05

/*
    r_socket, r_bind, r_sendto, r_recvfrom, r_close
*/

int r_socket(int domain, int type, int protocol);
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags,
                 const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags,
                   struct sockaddr *src_addr, socklen_t *addrlen);
int r_close(int fd);

int dropMessage(float p);

#endif