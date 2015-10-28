/*
    C socket server example
*/

#include "server_headers.h"

void start_server(char* port);

int main(int argc , char *argv[])
{
    if(argc != 2){
        printf("wrong usage\n");
        printf("%s port_number\n", argv[0]);
        return 2;
    }
    char * port;
    port = argv[1];
    start_server(port);
}

void sigchld_handler(int s){
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void set_hints(struct addrinfo* hints){
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE;
}

void start_server(char * port){
    int sockfd, new_fd;
    int numbytes;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    char recv_buf[MAXDATASIZE];
    memset(recv_buf, 0, sizeof(recv_buf));
    char s[INET6_ADDRSTRLEN];
    int rv;
    int yes = 1;
    int set_sock_val = 0;
    int bind_val = 0;
    int listen_val = 0;
    memset(&hints, 0, sizeof hints);
    set_hints(&hints);
    
    rv = getaddrinfo(NULL, port, &hints, &servinfo);
    if (rv != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    p = servinfo;
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    set_sock_val = 
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(set_sock_val == -1){
        perror("setsockopt");
        exit(1);
    }
    bind_val = bind(sockfd, p->ai_addr, p->ai_addrlen);
    if(bind_val == -1){
        close(sockfd);
        perror("server: bind");
    }
    if(p == NULL){
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    freeaddrinfo(servinfo);
    /* 
     * from now on, another function
     * */
    listen_val = listen(sockfd, BACKLOG);
    if(listen_val == -1){
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }
    printf("server: waiting for connections\n");
    sin_size = sizeof their_addr;

    int counter = 0;
    while(1){
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); 
        numbytes = recv(new_fd, recv_buf, MAXDATASIZE-1, 0);
        printf("counter: %d\n", counter);
        counter += 1;
        if (new_fd == -1){
            perror("accept");
            exit(1);
        }
        inet_ntop(their_addr.ss_family, 
                get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);
        while(numbytes > 0){
            numbytes = recv(new_fd, recv_buf, MAXDATASIZE-1, 0);
            printf("%s", recv_buf);
            /*printf("numbytes: %d\n", numbytes);*/
            if (numbytes == -1){
                perror("recv");
                exit(1);
            }
            recv_buf[numbytes] = '\0';
            memset(recv_buf, '\0', MAXDATASIZE);
        }
        /*printf("server: received %s\n", recv_buf);*/
    }
}
