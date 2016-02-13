#include "test.h"

static void* run_stream(void *params);
static void *run_receive(void *params);
static void* run_rx_proxy(void *params); 
static void* run_rx_queue(void *params);
static void execute_boss(char *dest_ip, char *dest_port);
static void test_boss(char *pxy_port, char *pxy_ip);
static int* generate_index(int size);
static bool isvalue_inarray(int val, int *arr, int size);
static void fill_packet(int seq_number, encaps_packet_t *packet);

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
    queue_args_t *queue_args;
    pthread_t rx_id, boss_id, 
              queue_id, stream_id;
    
    // get argument variables
    dest_port = argv[1];
    dest_ip = argv[2];
    rx_proxy_port = argv[3];
    output = argv[4];
    
    // set parameters
    fp = fopen(output, "a");
    pxy_rcv_sock = rcv_sock_init(rx_proxy_port);
    pxy_fwd_sock = fwd_sock_init(dest_ip, dest_port);
    fq = fqueue_init(pxy_fwd_sock);
    pl = pool_init();
    
    // load parameters to thread args 
    rx_args = (struct rx_params*) 
        malloc(sizeof(struct rx_params));
    rx_args->fp = fp;
    rx_args->port = dest_port;

    pxy_args = (struct pxy_params*)
        malloc(sizeof(struct pxy_params));
    pxy_args->pl = pl;
    pxy_args->recv_sock = pxy_rcv_sock;
    
    queue_args = (queue_args_t *)
        malloc(sizeof(queue_args_t));
    queue_args->pl = pl;
    queue_args->fq = fq;

    stream_args = (struct stream_params*)
        malloc(sizeof(struct stream_params));
    stream_args->rx_pxy_port = rx_proxy_port;
    stream_args->rx_pxy_ip = dest_ip;
    
    // start threads
    pthread_create(&rx_id, NULL, &run_receive, rx_args);
    pthread_create(&boss_id, NULL, &run_rx_proxy, pxy_args);
    pthread_create(&queue_id, NULL, &run_rx_queue, queue_args);
    pthread_create(&stream_id, NULL, &run_stream, stream_args);

    return 0;
}

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
    queue_args_t* queue_args = (queue_args_t*) params;
    wait2forward(queue_args);
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
        while (byte < BLOCKSIZE) {
            nwrite = write(sockfd, packets[i], BLOCKSIZE - byte);
        }
        if (nwrite < 0) {
            perror("Error");
            exit(1);
        } else if (nwrite > 0) {
            byte += nwrite;
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


