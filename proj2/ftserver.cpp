// Jason Dorweiler
// CS372 project 1, ftserver
//
// Desc: start an ftp server on a given port
// compile: make
// usage ./ftserver <port number>
//
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
#include <cstdio>
#include <vector>
#include <fstream>

#define BACKLOG 10     // how many pending connections queue will hold
using namespace std;
int quit = 0;
char cmd_args[4][100];

int setup(int sockfd, struct addrinfo *p, struct sigaction sa, struct addrinfo *servinfo, int yes) {
    for(p = servinfo; p != NULL; p = p->ai_next) {

        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

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

void sendmsg(int new_fd, vector<char> &msg){
    cout << "Sending..."<< endl;
    send(new_fd, &msg[0], msg.size(), MSG_CONFIRM);
}

// http://advancedcppwithexamples.blogspot.com/2011/03/reading-files-into-vector.html
void sendfile(int new_fd, int data_fd){
    ifstream file;
    char n;
    file.open(cmd_args[4]);

    if(!file.is_open()){
        string msg = "FILE NOT FOUND";
        send(new_fd, &msg[0], msg.size(), MSG_CONFIRM);
        return;
    }

    printf("Sending file: %s\n", cmd_args[4]);

    string line;
    while(getline(file, line)){
        line.append("\n");
        send(data_fd, &line[0], line.size(), MSG_CONFIRM);
    }

    file.close();
}

// Check for command type, -l (list), -g <filename> (send file),
// http://en.cppreference.com/w/cpp/io/c/fread
void receive(int new_fd, int data_fd){

    if(strncmp(cmd_args[3], "-l", 2) == 0){
        cout << "sending file list" << endl;
        FILE *dir = popen("ls", "r");
        vector<char> buff(500);

        fread(&buff[0], sizeof buff[0], buff.size(), dir);
        sendmsg(new_fd, buff);
    }
    else if(strncmp(cmd_args[3], "-g", 2) == 0){
        sendfile(new_fd, data_fd);
    }
}

// get the args from the client connection
void get_cmd_args(int new_fd){
    static char msg[500];
    msg[ recv(new_fd, msg, 500, 0) ] = '\0';
    char *split;
    split = strtok(msg, "\n");

    int i = 0;
    while (split != NULL){
        strcpy( cmd_args[i], split);
        split = strtok(NULL, "\n");
        i++;
    }
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

bool connected(int new_fd)
{
     char buf;
     int err = recv(new_fd, &buf, 0, MSG_PEEK);
     if(err == ENOTCONN){
        printf("socket closed\n");
        close(new_fd);
        quit = 1;
        return false;
     }
     return true;
}

int main(int argc, char **argv){
    int sockfd, datasock, new_fd, data_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int size = 500;
    char message_buffer[size];

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
    
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);       
    printf("server: got connection from %s\n", s); 
    
    get_cmd_args(new_fd);

    if ((rv = getaddrinfo(NULL, cmd_args[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    datasock = setup(datasock, p, sa, servinfo, yes);
    data_fd = accept(datasock, (struct sockaddr *)&their_addr, &sin_size);
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);       

    while(1){

        receive(new_fd, data_fd);
        close(new_fd);
        close(data_fd);
        new_fd = data_fd = 0;

        if(new_fd == 0){
            new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
            inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);       
            printf("server: got connection from %s\n", s); 
            
            get_cmd_args(new_fd);
    
            data_fd = accept(datasock, (struct sockaddr *)&their_addr, &sin_size);
            inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);       
        }
    }
}
