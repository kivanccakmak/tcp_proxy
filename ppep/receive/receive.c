#include "receive.h"

static void get_packets(char *port, FILE *fp);

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
 * @brief 
 *
 * @param[in] port
 * @param[in] fp
 */
static void get_packets(char *port, FILE *fp) 
{ 
    int rs_addr;
    int sockfd;
    int yes = 1;
    int n = 1, i = 0;
    int recv_count = 0;
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

    while (n > 0) {
        memset(buffer, '\0', sizeof(buffer) - 1);
        while (recv_count < BLOCKSIZE) {
            n = recv(sockfd, buffer+recv_count, BLOCKSIZE-recv_count, 0);
            if (n > 0) {
                recv_count += n;
            } else {
                break;
            }
        }
        if (n < 1) {
            break;
        }
        recv_count = 0;
        fprintf(fp, "%s", buffer);
    }
    printf("after while loop\n");
    printf("buffer: %s\n", buffer);
    printf("recv_count: %d\n", recv_count);
    if (recv_count > 0) {
        for (i = 0; i < recv_count; i++) {
            if (buffer[i] != EOF) {
                fprintf(fp, "%c", buffer[i]);
            }
            printf("buffer[%d]:%c\n", i, buffer[i]);
        }
        /*fprintf(fp, "%*s", recv_count, buffer);*/
    }
    fclose(fp);
}


