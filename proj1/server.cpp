// These sources were helpful:
// http://beej.us/guide/bgnet/output/html/multipage/syscalls.html#socket
// https://www.youtube.com/watch?v=IydkqseK6oQ
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>

#define BACKLOG 10     // how many pending connections queue will hold
using namespace std;
int quit = 0;

void handler(int s){
    if(s == SIGPIPE){
        quit = 1;
    }
    else {

        exit(s);
    }
}

int setup(int sockfd, struct addrinfo *p, struct sigaction sa, struct addrinfo *servinfo, int yes) {

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {

        // get a socket descriptor
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        // 
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // bind socket to port
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    return sockfd;
}

void sendmsg(int new_fd){
    char buffer[500];
    int length;

    fgets(buffer, 500, stdin);
    length = strlen(buffer);
    buffer[length-1] = '\n';

    if( strncmp(buffer, "\\quit", 5) == 0){
        quit = 1;
    }

    if (send(new_fd, buffer, length, 0) == -1){
        perror("send");
    }
}

char *receive(int new_fd){
    static char msg[500];
    msg[ recv(new_fd, msg, 500, 0) ] = '\0';

    if( strncmp(msg, "\\quit", 5) == 0 ){
        quit = 1;
    }

    printf("%s", msg);
    fflush(stdout);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char **argv){
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int size = 500;
    char message_buffer[size];

    signal(SIGPIPE, handler);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // get all types back
    hints.ai_socktype = SOCK_STREAM; // type SOCK_STREAM, SOCK_DGRAM
    hints.ai_flags = AI_PASSIVE; // use my IP

    if (argc < 2){
        std::cerr << "Usage: " << argv[0] << "<port number>" << std::endl;
        return 1;
    }

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    sockfd = setup(sockfd, p, sa, servinfo, yes);

    printf("server: waiting for connections on %s...\n", argv[1]);
    printf("sockfd: %d\n", sockfd);
    
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);       
    printf("server: got connection from %s\n", s); 
    strcpy(message_buffer, "Enter your name: ");
    send(new_fd, message_buffer, 17, 0);
    receive(new_fd);

    while(1){

        while(new_fd > 0) {

            while(!quit){

                if(!quit){
                    cout << "you > ";
                    sendmsg(new_fd);
                }            

                if(!quit){
                    receive(new_fd);
                }
            }

            close(new_fd);

            new_fd = 0;

            if(new_fd == 0){
                cout << "CONNECTION CLOSED \n" << endl;
                quit = 0;
                new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
                inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);       
                printf("server: got connection from %s\n", s); 
                strcpy(message_buffer, "Enter your name: ");
                send(new_fd, message_buffer, 17, 0);
                receive(new_fd);
            }
        }
    }
    return 0;
}