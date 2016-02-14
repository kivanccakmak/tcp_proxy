#include "test.h"

static void* run_stream(void *params);
static void *run_receive(void *params);
static void* run_rx_proxy(void *params); 
static void* run_rx_queue(void *params);
static void test_boss(char *pxy_port, char *pxy_ip);
static int* generate_index(int size);
static bool isvalue_inarray(int val, int *arr, int size);
static void fill_packet(int seq_number, encaps_packet_t *packet);

#ifdef TEST_RX_PROXY
/**
 * @brief main test function of receiver
 * side of pair performance enhancing
 * proxy.  
 *
 * @param[in] argc
 * @param[in] argv[]
 *
 * @return 
 */
int main(int argc, char *argv[])
{
    if (argc != 5) {
        printf("usage: %s dest_port dest_ip proxy_port"
                "out_file\n", argv[0]);
        printf("ex: 6060 127.0.0.1 5050 output.txt\n");
        exit(0);
    }

    char *dest_port, *dest_ip, *rx_proxy_port; 
    char *output;
    int pxy_rcv_sock, pxy_fwd_sock;
    FILE *fp;
    pool_t *pl;
    fqueue_t *fq;
    struct rx_params *rx_args;
    struct pxy_params *pxy_args;
    struct stream_params *stream_args;
    struct queue_params* queue_prm;
    pthread_t rx_id, boss_id, 
              queue_id, stream_id;
    
    // get argument variables
    dest_port = argv[1];
    dest_ip = argv[2];
    rx_proxy_port = argv[3];
    output = argv[4];
    
    fp = fopen(output, "a");

    // init receive thread
    printf("==========================\n");
    printf("init receive thread \n");
    printf("==========================\n");
    pxy_rcv_sock = rcv_sock_init(rx_proxy_port);
    rx_args = (struct rx_params*) 
        malloc(sizeof(struct rx_params));
    rx_args->fp = fp;
    rx_args->port = dest_port;
    pthread_create(&rx_id, NULL, &run_receive, rx_args);
    sleep(3);

    pxy_fwd_sock = fwd_sock_init(dest_ip, dest_port);
    fq = fqueue_init(pxy_fwd_sock);
    fq->byte_capacity = 41;
    printf("fq->byte_capacity: %d\n", fq->byte_capacity);
    pl = pool_init();

    // init boss server thread
    printf("==========================\n");
    printf("init run proxy thread \n");
    printf("==========================\n");
    pxy_args = (struct pxy_params*)
        malloc(sizeof(struct pxy_params));
    pxy_args->pl = pool_init();
    pxy_args->recv_sock = pxy_rcv_sock;
    pthread_create(&boss_id, NULL, &run_rx_proxy, pxy_args);

    // init fwd queue thread
    printf("==========================\n");
    printf("init fwd_queue thread \n");
    printf("==========================\n");
    printf("%d\n", pl->pq->size);
    printf("%d\n", pl->pq->min_seq);
    queue_prm = (struct queue_params*) 
        malloc(sizeof(struct queue_params));
    queue_prm->pl = pl;
    queue_prm->fq = fq;
    pthread_create(&queue_id, NULL, &run_rx_queue, (void *) queue_prm);
    
    // init stream thread
    printf("==========================\n");
    printf("init stream thrad\n");
    printf("==========================\n");
    stream_args = (struct stream_params*)
        malloc(sizeof(struct stream_params));
    stream_args->rx_pxy_port = rx_proxy_port;
    stream_args->rx_pxy_ip = dest_ip;
    pthread_create(&stream_id, NULL, &run_stream, stream_args);
    
    pthread_join(rx_id, NULL);
    pthread_join(queue_id, NULL);
    pthread_join(boss_id, NULL);
    pthread_join(stream_id, NULL);

    return 0;
}
#endif

static void* run_stream(void *params)
{
    struct stream_params* stream_ptr =
        (struct stream_params*) params;
    test_boss(stream_ptr->rx_pxy_port,
            stream_ptr->rx_pxy_ip);
    return NULL;
}

/**
 * @brief run queue side of receiving
 * proxy, which would be nudged by
 * receiving threads and forwards 
 * data towards end-destination.
 *
 * @param params
 *
 * @return 
 */
static void* run_rx_queue(void *params)
{
    wait2forward(params);
    return NULL;
}

/**
 * @brief run receiver side
 * of proxy in seperate thread
 *
 * @param params
 *
 * @return 
 */
static void* run_rx_proxy(void *params) 
{
    struct pxy_params* pxy_ptr = 
        (struct pxy_params*) params;
    server_listen(pxy_ptr->recv_sock, pxy_ptr->pl);
    return NULL;
}

/**
 * @brief run receiving function
 * in seperate thread that models
 * agnostic end destination
 *
 * @param params
 *
 * @return 
 */
static void* run_receive(void *params) {
    struct rx_params* rx_ptr = (struct rx_params*) params; 
    printf("running get_packets()\n");
    printf("port: %s", rx_ptr->port);
    get_packets(rx_ptr->port, rx_ptr->fp);
    return NULL;
}

/**
 * @brief assumes that boss_server 
 * and receive modules are running.
 * Then, initiates randomized 
 * tx_packets towards boss_server
 * module.
 *
 * @param[in] pxy_port
 * @param[in] pxy_ip
 */
static void test_boss(char *pxy_port, char *pxy_ip)
{
    int *ind_array;
    int i = 0;
    int sockfd;
    int byte = 0, nwrite = 0;

    encaps_packet_t **packets = NULL;
    encaps_packet_t *temp = NULL;
    encaps_packet_t packet;

    packets = (encaps_packet_t **)
        malloc(sizeof(encaps_packet_t*) * MAX_PACKET);
    ind_array = generate_index(MAX_PACKET);

    for (i = 0; i < MAX_PACKET; i++) {
        temp = (encaps_packet_t *) malloc(sizeof(encaps_packet_t*));
        fill_packet(ind_array[i], temp);
        printf("seq: %d, raw: %s \n", 
                ind_array[i], temp->raw_packet);
        packets[i] = temp;
    }
    
    sockfd = fwd_sock_init(pxy_ip, pxy_port);

    for (i = 0; i < MAX_PACKET; i++) {
        memcpy(packet.raw_packet, packets[i]->raw_packet, BLOCKSIZE);
        packet.seq = (unsigned short) packets[i]->seq;
        while (byte < BLOCKSIZE) {
            nwrite = send(sockfd, 
                    &((unsigned char*) &packet)[byte], 
                    PACKET_SIZE-byte, 0);
            if (nwrite < 0) {
                perror("Error");
                exit(1);
            } else if (nwrite > 0) {
                byte += nwrite;
            }
        }
        byte = 0;
    }
}

/**
 * @brief 
 *
 * Generates integer array, that has
 * random and unique integers.
 *
 * Such that, for input size=4,
 * output = [0, 2, 1, 3] etc.
 *
 * @param[in] size
 *
 * @return 
 */
static int* generate_index(int size) 
{
    int count = 0, val;
    int *index_vals = NULL;
    index_vals = (int *) 
        malloc(sizeof(int*) * size);
    bool check_exist;

    while (count < size) {
        val = rand() % size;
        if (count == 0) {
            index_vals[count] = val;
            count++;
        } else{
            check_exist = isvalue_inarray(val, 
                    index_vals, count);
            if (check_exist == false){
                index_vals[count] = val;
                count++;
            }
        }
    }
    return index_vals;
}

/**
 * @brief check whether array 
 * contains integer
 *
 * @param[in] val
 * @param[in] arr
 * @param[in] size
 *
 * @return 
 */
static bool isvalue_inarray(int val, int *arr, 
        int size)
{
    int i;
    for (i = 0; i < size; i++) {
        if (arr[i] == val) {
            return true;
        }
    }
    return false;
}

/**
 * @brief generate bursty 
 * encapsulated packet
 *
 * @param[in] seq_number
 * @param[out] packet
 */
static void fill_packet(int seq_number, 
        encaps_packet_t *packet)
{
    int i = 0;
    char randomletter = 'A' + (rand() % 26);
    packet->seq = seq_number;

    for (i = 0; i < BLOCKSIZE; i++)
        packet->raw_packet[i] = randomletter;
}
