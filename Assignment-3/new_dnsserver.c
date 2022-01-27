// Assignment - 02.(b)
// new_dnsserver.c
// Vanshita Garg
// 19CS10064

// A Concurrent Server implementation
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>
  
const int MAX = 1000; // MAX dnsName LENGTH TO RECEIVE FROM CLIENT

// Auxilliary function to compute maximum of two integers
int max (int a, int b) {
    return ((a > b) ? a : b);
}

                /* THE SERVER PROCESS */
int main() { 
    int UDPsockfd, TCPsockfd, newsockfd;  /* Socket descriptor */
    struct sockaddr_in servaddr, cliaddr; 
    char dnsName[MAX];  /* We will use this dnsName for communication */
      
    /* The following system call opens a socket. The first parameter
       indicates the family of the protocol to be followed. For internet
       protocols we use AF_INET. For TCP sockets the second parameter
       is SOCK_DGRAM. The third parameter is set to 0 for user
       applications.
    */ 

    // Creating UDP socket file descriptor 
    UDPsockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( UDPsockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    // Creating TCP socket file descriptor 
    TCPsockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( TCPsockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    /* The structure "sockaddr_in" is defined in <netinet/in.h> for the
       internet family of protocols. This has three main fields. The
       field "sin_family" specifies the family and is therefore AF_INET
       for the internet family. The field "sin_addr" specifies the
       internet address of the server. This field is set to INADDR_ANY
       for machines having a single IP address. The field "sin_port"
       specifies the port number of the server.
    */
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
      
    // Bind the UDP socket with the server address 
    if ( bind(UDPsockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )  { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    // Bind the TCP socket with the server address 
    if (bind(TCPsockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Unable to bind local address\n");
        exit(0);
    }

    /*  This specifies that up to 5 concurrent client
        requests will be queued up while the system is
        executing the "accept" system call below.
    */
    listen(TCPsockfd, 5);
    
    /* In this program we are illustrating a concurrent server -- one
       which forks to accept multiple client connections concurrently.
       As soon as the server accepts a connection from a client, it
       forks a child which communicates with the client, while the
       parent becomes free to accept a new connection. To facilitate
       this, the accept() system call returns a new socket descriptor
       which can be used by the child. The parent continues with the
       original socket descriptor.
    */
    while (1) {
        printf("\nServer Waiting for the client connection...\n\n");
        fd_set fd;
        FD_ZERO(&fd);
        FD_SET(TCPsockfd, &fd);
        FD_SET(UDPsockfd, &fd);

        // Select call to select to socket on which data is received.
        int bitsSet = select (max(TCPsockfd, UDPsockfd) + 1, &fd, NULL, NULL, NULL);
        
        // If select call returns < 0, unable to select
        if (bitsSet < 0) {
            perror("Unable to Select\n"); 
            exit(0);
        }

        // if TCPsockfd bit is set in fd, data received on TCP socket
        if (FD_ISSET(TCPsockfd, &fd)) {
            socklen_t clilen = sizeof(cliaddr);
            memset(&cliaddr, 0, sizeof(cliaddr)); 
            newsockfd = accept(TCPsockfd, (struct sockaddr *) &cliaddr, &clilen);
            if (newsockfd < 0) {
                printf("Accept error\n");
                exit(1);
            }
            /* Having successfully accepted a client connection, the
               server now forks. The parent closes the new socket
               descriptor and loops back to accept the next connection.
            */  
            if (fork() == 0) {

                /* This child process will now communicate with the
                   client through the send() and recv() system calls.
                */
                close(TCPsockfd);
                close(UDPsockfd);  /* Close the old sockets since all
                           communications will be through
                           the new socket.
                        */

                /* We initialize the buffer, copy the message to it,
                   and send the message to the client. 
                */
                // receive the domain name
                char buf[MAX];
                memset (dnsName, '\0', sizeof(dnsName));
                while (1) {
                    memset(buf, '\0', sizeof(buf));
                    int length = recv (newsockfd, buf, MAX, 0);
                    // length is less than zero if server is unable to receive from the client.
                    if (length < 0) {
                        perror ("Unable to receive from socket\n");
                        exit (1);
                    } 
                    // length is zero if connection is closed by the client.
                    else if (length == 0) {
                        perror ("Connection closed by client\n");
                        exit (1);
                    } 
                    // length is >0 if message is received from the client.
                    else {
                        strcat(dnsName, buf);
                        int exit = 0;
                        for (int i = 0; i < length; i++) {
                            if (buf[i] == '\0') {
                                exit = 1;
                                break;
                            }
                        }
                        if (exit) {
                            break;
                        }
                    }
                }
                // Send response to the client
                struct hostent *h;
                char IPAddress[MAX];
                if ((h = gethostbyname(dnsName)) == NULL) {
                    memset(IPAddress, '\0', sizeof(IPAddress));
                    // Send special address when no IP address can be found
                    strcpy(IPAddress, "0.0.0.0");
                    send(newsockfd, IPAddress, strlen(IPAddress) + 1, 0); 
                } else {
                    // Send IP addresses to the client one after another
                    for(int i = 0; h->h_addr_list[i] != NULL; i++) {
                        memset(IPAddress, '\0', sizeof(IPAddress));
                        strcpy(IPAddress, inet_ntoa(*((struct in_addr *)h->h_addr_list[i])));
                        send(newsockfd, IPAddress, strlen(IPAddress) + 1, 0); 
                    }
                    // Send '\0' to detect the end of receive calls on client side.
                    send(newsockfd, "\0", 1, 0); 
                }
                // Close newsockfd
                close(newsockfd);
                // exit the child process
                exit(0);
            }
            close(newsockfd);
        } 
        // if UDPsockfd bit is set in fd, data received on UDP socket
        else if (FD_ISSET(UDPsockfd, &fd)) {
            socklen_t len = sizeof(cliaddr);
            memset (dnsName, '\0', sizeof(dnsName));
            int length = recvfrom(UDPsockfd, dnsName, MAX, 0, ( struct sockaddr *) &cliaddr, &len); 
            // length is less than zero if server is unable to receive from the client.
            if (length < 0) {
                perror ("Unable to receive from socket\n");
                exit (1);
            } 
            // length is zero if connection is closed by the client.
            else if (length == 0) {
                perror ("Connection closed by client\n");
                exit (1);
            } 
            // length is >0 if message is received from the client.
            else {
                struct hostent *h;
                char IPAddress[MAX];
                if ((h = gethostbyname(dnsName)) == NULL) {
                    memset(IPAddress, '\0', sizeof(IPAddress));
                    // Send special address when no IP address can be found
                    strcpy(IPAddress, "0.0.0.0");
                    sendto(UDPsockfd, IPAddress, MAX, 0, ( struct sockaddr *) &cliaddr, len); 
                } else {
                    // Send IP addresses to the client one after another
                    for(int i = 0; h->h_addr_list[i] != NULL; i++) {
                        sendto(UDPsockfd, inet_ntoa(*((struct in_addr *)h->h_addr_list[i])), MAX, 0, ( struct sockaddr *) &cliaddr, len); 
                    }
                }
            }
        }
    }
    // Close the socket
    close(UDPsockfd);
    close(TCPsockfd);
    return 0; 
} 
