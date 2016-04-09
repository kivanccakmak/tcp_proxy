#include "receive.h"

static void recv_loop(struct pollfd *pfd, int sockfd, 
        FILE *fp, FILE *logp);

static void get_packets(char *port, FILE *fp, FILE *logp);

static int sock_init(char *port);

#ifdef RECV
int main(int argc, char *argv[]) 
{
    if (argc != 4) {
        printf("Wrong usage\n");
        printf("%s port_number output_file log_file\n", argv[0]);
        return 0;
    }

    char *port;
    char *output;
    char *log_file;
    FILE *fp, *logp;

    port = argv[1];
    output = argv[2];
    log_file = argv[3];

    fp = fopen(output, "a");
    logp = fopen(log_file, "a");

    get_packets(port, fp, logp);
    return 0;
}
#endif

/**
 * @brief sets socket file descriptor
 * to receive stream
 *
 * @param[in] port
 */
static int sock_init(char *port)
{
    int sockfd, rs_addr, yes = 1;
    
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
 * @brief 
 *
 * @param[in] pfd
 * @param[in] sockfd
 * @param[in] fp
 * @param[in] logp
 */
static void recv_loop(struct pollfd *pfd, int sockfd, 
        FILE *fp, FILE *logp)
{
    int numbytes = 0, recv_count = 0,
        total = 0, i = 0;
    char buffer[BLOCKSIZE];
    char log_str[100];
    clock_t t1, t2;
    float t_diff;

    t1 = clock();

    // receive until connection closed
    while (true) {
        if (poll(pfd, 1, 100) > 0) {
            recv_count = 0;
            if (pfd->revents == POLLIN) {
                memset(buffer, '\0', BLOCKSIZE);
                while (recv_count < BLOCKSIZE) {
                    numbytes = recv(sockfd, buffer+recv_count,
                            BLOCKSIZE-recv_count, 0);
                    if (numbytes > 0) {
                        recv_count += numbytes;
                        total += numbytes;
                    } else if (numbytes == 0)
                        goto CLOSE_CONN;
                }
                if (recv_count > 0) {
                    t2 = clock();
                    t_diff = ((float) (t2 - t1) / 1000000) * 1000;
                    sprintf(log_str, "diff: %f bytes: %d\n",
                            t_diff, total);
                    fprintf(fp, "%s", buffer);
                    fprintf(logp, "%s", log_str);
                    fflush(fp);
                    fflush(logp);
                }
            }
        }
    }
CLOSE_CONN:
    if (recv_count > 0) {
        total += recv_count;
        t2 = clock();
        t_diff = ((float) (t2 - t1) / 1000000) * 1000;
        sprintf(log_str, "diff: %f bytes: %d\n",
                t_diff, total);
        fprintf(logp, "%s", log_str);
        for (i = 0; i < recv_count; i++) {
            if (buffer[i] != EOF) {
                fprintf(fp, "%c", buffer[i]);
                fflush(fp);
            }
        }
    }
    fclose(fp);
}

/**
 * @brief receive packets and
 * records them into file
 *
 * @param[in] port
 * @param[in] fp
 * @param[in] logp
 */
void get_packets(char *port, FILE *fp, FILE *logp) 
{ 
    int sockfd;
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

    recv_loop(&pfd, sockfd, fp, logp);

}
