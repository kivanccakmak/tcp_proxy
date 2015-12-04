#include "split.h"

#define QUEUE_NUM 0 

static int nfqueue_get_syn(struct nfq_q_handle *qh,
        struct nfgenmsg *nfmsg, struct nfq_data *nfa,
        void *data);

static void *queuer_loop(void __attribute__((unused)) *unused);

static void *pass_payload(void *args);

static struct listen_args* set_listen_args(
        char *dest_ip, char *dest_port, char *local_part);

static int set_tx_sock(char *dest_ip, char *dest_port); 

int main(int argc, char *argv[])
{

    if (argc != 4) {
        printf("wrong usage \n");
        printf("%s local_port dest_ip dest_port\n", argv[0]);
        return 0;
    } 

    int ret;
    char *dest_ip, *dest_port, *local_port;
    struct listen_args *proxy;
    pthread_t queuer, passer;

    ret = pthread_create(&queuer, NULL, queuer_loop, NULL);
    if (ret) {
        printf("Failed to create queuer thread! [RET = %d]", ret);
    }

    local_port = argv[1];
    dest_ip = argv[2];
    dest_port = argv[3];
    proxy = set_listen_args(dest_ip, dest_port, local_port);

    ret = pthread_create(&passer, NULL, 
            pass_payload, (void *) proxy);

    pthread_join(queuer, NULL);
    pthread_join(passer, NULL);
    return 0;
}

/**
 * @brief 
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 * @param[in] local_port
 *
 * @return 
 */
static struct listen_args* set_listen_args(char *dest_ip, char *dest_port,
        char *local_port) {

    struct listen_args *proxy = 
        (struct listen_args*) malloc(sizeof(struct listen_args*));

    proxy->local_port = local_port;
    proxy->dest_port = dest_port;
    proxy->dest_ip = dest_ip;

    return proxy;
}

static void *queuer_loop(void __attribute__((unused)) *unused)
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	struct nfnl_handle *nh;

	int *sockfd, fd, i, ret;
	char buf[QUEUER_BUF_SIZE];
    ssize_t rc;
    char *dest_ip, *dest_port;

    dest_ip = (char *) malloc(20);
    dest_port = (char *) malloc(20);
    sockfd = (int *) malloc(sizeof(int*));

    strcpy(dest_ip, "192.168.2.202");
    strcpy(dest_port, "5050");
    /*set_conn(dest_ip, dest_port, sockfd);*/

    h = nfq_open();
    if (!h) {
        printf("Failed to open NFQ handler! \n");
    }

    printf("unbinding existing nf_queue handler "
              "for AF_INET (if any) \n");
    nfq_unbind_pf(h, AF_INET);

    printf("binding nfnetlink_queue as "
              "nf_queue handler for AF_INET\n");
    ret = nfq_bind_pf(h, AF_INET);
    if (ret < 0) {
        printf("Failed to bind NFQ handler! [RET = %d] \n", ret);
        exit(0);
    }

    printf("binding this socket to queue '%d'", QUEUE_NUM);
    qh = nfq_create_queue(h, QUEUE_NUM, &nfqueue_get_syn, NULL);
    if (!qh) {
        printf("Failed to create nfnetlink queue!\n");
        exit(0);
    }

    printf("setting copy_packet mode");
    ret = nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);
    if (ret < 0) {
        printf("Failed to setup NFQ packet_copy mode! [RET = %d]\n", ret);
        exit(0);
    }

    nh = nfq_nfnlh(h);
    fd = nfnl_fd(nh);

    while ((rc = recv(fd, buf, QUEUER_BUF_SIZE, 0)) >= 0) {
        printf("received packet [sz = %d]\n", (int) rc);
        nfq_handle_packet(h, buf, rc);
        for (i = 0; i < (int) rc;  i++) {
            printf("%c", (char) buf[i]);
        }
    }

    printf("Exiting [rc=%d]\n", (int) rc);
    nfq_close(h);
    pthread_exit(NULL);
}

/**
 * @brief Reads local socket and 
 * passes through the end destination
 *
 * @param[in] args
 *
 * @return 
 */
void *pass_payload(void *args) 
{
    int rs_addr, rb, wb, numbytes;
    int send_count = 0, recv_count = 0;
    int rv, yes = 1, set_val, bind_val, listen_val;
    struct listen_args *proxy;
    struct addrinfo hints;
    struct addrinfo *addr;
    struct sockaddr_storage their_addr;
    struct pollfd pfd;
    socklen_t sin_size;
    int split_sock, tx_sock;
    char src_ip[INET6_ADDRSTRLEN];
    uint32_t src_port;
    char *raw_buf = 
        (char*) malloc(BUFF_SIZE);

    proxy = (struct listen_args*) 
        malloc(sizeof(struct listen_args*));
    proxy = (struct listen_args*) args;

    tx_sock = set_tx_sock(proxy->dest_ip, 
            proxy->dest_port);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rs_addr = getaddrinfo(NULL, proxy->local_port, 
            &hints, &addr);
    split_sock = socket(addr->ai_family, addr->ai_socktype,
            addr->ai_protocol);
    set_val = 
        setsockopt(split_sock, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int));

    if (addr == NULL) {
        fprintf(stderr, "server: failed to bind \n");
        exit(1);
    }

    bind_val = bind(split_sock, addr->ai_addr, addr->ai_addrlen);
    if (bind_val == -1) {
        close(split_sock);
        perror("server: bind");
    }

    listen_val = listen(split_sock, 1);
    sin_size = sizeof(their_addr);

    while (1) {
        split_sock = accept(split_sock, (struct sockaddr*)&their_addr,
                &sin_size);
        pfd.fd = split_sock;
        pfd.events = POLLIN;
        inet_ntop(their_addr.ss_family, 
                get_in_ipaddr((struct sockaddr*)&their_addr),
                src_ip, sizeof(src_ip));
        src_port = get_in_portnum((struct sockaddr *)&their_addr);
        printf("split: got connection from %s : %d \n", 
                src_ip, src_port);
        while (1) {
            rv = poll(&pfd, 1, -1);
            while (recv_count < (int) BUFF_SIZE) {
                numbytes = recv(split_sock, raw_buf+recv_count, 
                        BUFF_SIZE-recv_count, 0);
                if (numbytes > 0) {
                    recv_count += numbytes;
                }
            }
            printf("recv_count: %d\n", recv_count);
            printf("raw_buf: %s\n", raw_buf);
            recv_count = 0;
            numbytes = 0;
            while (send_count < (int) BUFF_SIZE) {
                numbytes = write(tx_sock, raw_buf+send_count,
                        BLOCKSIZE-send_count);
                if (numbytes > 0) {
                    send_count += numbytes;
                }
                printf("numbytes: %d\n", numbytes);
                if (numbytes == 0) {
                    break;
                }
            }
            send_count = 0;
            raw_buf = (char *) malloc(BUFF_SIZE);
        }

    }
}


/**
 * @brief  
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 *
 * @return 
 */
static int set_tx_sock(char *dest_ip, char *dest_port) 
{
    int sockfd;
    int conn_res;

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(dest_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(dest_port));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    conn_res = connect(sockfd, (struct sockaddr*)&server,
            sizeof(server));

    if (conn_res < 0) {
        perror("connection failed. Error");
        exit(0);
    }

    return sockfd;

}

static int nfqueue_get_syn(struct nfq_q_handle *qh,
        struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data) 
{
    unsigned char *buffer;
    struct ipv4_packet *ip4;
	int id = 0, ret;
	struct nfqnl_msg_packet_hdr *ph;

    printf("**********************\n");
    printf("in nfqueue_get_syn\n");
    printf("**********************\n");

    ph = nfq_get_msg_packet_hdr(nfa);
    if (!ph) {
        perror("unable to get packet header");
    }

    id = ntohl(ph->packet_id);
    ret = nfq_get_payload(nfa, &buffer);

    ip4 = (struct ipv4_packet *) buffer;

    ret = nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL); 

    if (ret < 0) {
        printf("nfq_set_verdict to NF_ACCEPT failed\n");
    } else{
        printf("accepted \n");
    }

    return 0;
}
