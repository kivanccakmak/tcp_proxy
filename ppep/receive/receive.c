#include "receive.h"

#ifdef RECV
int main(int argc, char *argv[]) 
{
    if (argc != 3) {
        printf("Wrong usage\n");
        printf("%s port_number output_file\n", argv[0]);
        return 0;
    }

    char *port;
    char *output;
    FILE *fp;

    port = argv[1];
    output = argv[2];
    fp = fopen(output, "a");

    get_packets(port, fp);
    return 0;
}
#endif

/**
 * @brief sets socket file descriptor
 * to receive stream
 *
 * @param[in] port
 */
int sock_init(char *port)
{
    int sockfd, rs_addr;
    int yes = 1;
    
    struct addrinfo hints, *addr;

    addr = (struct addrinfo*) 
        malloc(sizeof(struct addrinfo*));

    // set hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rs_addr = getaddrinfo(NULL, port, &hints, &addr);
    if (rs_addr != 0) {
        fprintf(stderr,
                "getaddrinfo: %s\n", gai_strerror(rs_addr));
    }

    sockfd = socket(addr->ai_family,
            addr->ai_socktype, addr->ai_protocol);

    if (setsockopt(sockfd, SOL_SOCKET, 
                SO_REUSEADDR, &yes, sizeof(int)) != 0) {
        perror("setsockopt: ");
        exit(1);
    }

    if (bind(sockfd, addr->ai_addr, addr->ai_addrlen) == -1) {
        close(sockfd);
        perror("server: bind");
        exit(1);
    }

    if (addr == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    return sockfd;
}

/**
 * @brief receive packets and
 * records them into file
 *
 * @param[in] port
 * @param[in] fp
 */
void get_packets(char *port, FILE *fp) 
{ 
    int sockfd, numbytes = 0, i = 0, recv_count = 0;
    char buffer[BLOCKSIZE];
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct pollfd pfd;

    sin_size = sizeof(their_addr);
    sockfd = sock_init(port);

    printf("end destination listens \n");
    if (listen(sockfd, 1) != 0) {
        perror("listen: ");
        exit(1);
    }

    printf("end destination waits connection \n");
    sockfd = accept(sockfd, 
            (struct sockaddr*)&their_addr, &sin_size);
    printf("end destination accepted connection \n");

    pfd.fd = sockfd;
    pfd.events = POLLIN;
    
    // receive until connection closed
    while (true) {
        if (poll(&pfd, 1, 100) > 0) {
            recv_count = 0;
            if (pfd.revents == POLLIN) {
                memset(buffer, '\0', BLOCKSIZE);
                while (recv_count < BLOCKSIZE) {
                    numbytes = recv(sockfd, buffer+recv_count,
                            BLOCKSIZE-recv_count, 0);
                    if (numbytes > 0) 
                        recv_count += numbytes;
                    else if (numbytes == 0)
                        goto CLOSE_CONN;
                }
                if (recv_count > 0) {
                    fprintf(fp, "%s", buffer);
                    fflush(fp);
                }
            }
        }
    }
CLOSE_CONN:
    if (recv_count > 0) {
        for (i = 0; i < recv_count; i++) {
            if (buffer[i] != EOF) {
                fprintf(fp, "%c", buffer[i]);
                fflush(fp);
            }
        }
    }
    fclose(fp);
}
