#include "receive.h"

static void get_packets(char *port);

int main(int argc, char *argv[]) 
{

    if (argc != 2) {
        printf("Wrong usage\n");
        printf("%s port_number\n", argv[0]);
        return 0;
    }

    char *port;
    port = argv[1];
    get_packets(port);
    return 0;
}


/**
 * @brief 
 *
 * @param[in] port
 */
static void get_packets(char *port) 
{ 
    int rs_addr;
    int sockfd;
    int yes = 1;
    int n;
    int set_val, bind_val, listen_val;
    struct sockaddr_storage their_addr;
    struct addrinfo hints, *addr, *servinfo;
    socklen_t sin_size;
    char buffer[BLOCKSIZE];

    addr = (struct addrinfo*) 
        malloc(sizeof(struct addrinfo*));

    // set hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rs_addr = getaddrinfo(NULL, port,
            &hints, &addr);
    if (rs_addr != 0) {
        fprintf(stderr,
                "getaddrinfo: %s\n", gai_strerror(rs_addr));
    }

    sockfd = socket(addr->ai_family,
            addr->ai_socktype, addr->ai_protocol);

    set_val = setsockopt(sockfd, SOL_SOCKET,
            SO_REUSEADDR, &yes, sizeof(int));

    bind_val = bind(sockfd, addr->ai_addr, addr->ai_addrlen);

    if (bind_val == -1) {
        close(sockfd);
        perror("server: bind");
    }
    if (addr == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    sin_size = sizeof(their_addr);
    printf("end destination listens \n");
    listen_val = listen(sockfd, 1);

    printf("end destination waits connection \n");
    sockfd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
    printf("end destination accepted connection \n");

    while ((n = recv(sockfd, buffer, BLOCKSIZE, 0)) > 0) {
        printf("buf: %s \n", buffer);
    } 








}


