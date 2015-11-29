#include "test.h"

static void execute_queue(char *dest_ip, 
        char *dest_port);

static void execute_reorder(char *dest_ip,
        char *dest_port);

static void *test_queue(void *args);

static void *init_receive(void *args);

static bool isvalue_inarray(int val, 
        int *arr, int size); 

static int* generrate_index(int size); 

static void fill_packet(int seq_number,
        encaps_packet_t *packet);

/**
 * @brief 
 *
 * main test code. gets argv variable
 * to test units.
 *
 * @param argc
 * @param argv[]
 *
 * @return 
 */
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("usage: %s test_unit END_DEST_IP END_DEST_PORT \n", argv[0]);
        printf("ex: %s test_queue 127.0.0.1 5050 \n", argv[0]);
    } else{
        if (strcmp("-que", argv[1]) == 0) {
            execute_queue(argv[2], argv[3]);
        } else if(strcmp("-reord", argv[1]) == 0) {
            execute_reorder(argv[2], argv[3]);
        }
    }
    return 0;
}

/**
 * @brief 
 *
 * This function tests reorder thread. First, it
 * creates destination thread with init_receive().
 * Then creates queue thread with queue_wait().
 * Then creates reorder thread with nudge_queue().
 * Finally, passes unordered encapsulated packets
 * in casted encaps_packet_t format to packet_pool
 * by using push2pool().
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 */
static void execute_reorder(char *dest_ip, 
        char *dest_port) {
    clock_t start = clock(), diff;
    int msec;
    int i = 0;
    int dest_thr, queue_thr, pool_thr;
    int *ind_array;
    pthread_t dest_id, queue_id, pool_id;
    queue_t *que = NULL;
    struct packet_pool *pool = NULL;
    cb_reord_args_t *reord_args = NULL;
    struct debug_receiver_args *rx_args = NULL;

    rx_args = (struct debug_receiver_args *)
        malloc(sizeof(struct debug_receiver_args*));

    rx_args->filename = (char *) RECEIVER_OUT;
    rx_args->dest_port = dest_port;
    
    dest_thr = pthread_create(&dest_id, NULL, &init_receive,
            (void *) rx_args);
    sleep(5);
    que = queue_init(dest_ip, dest_port);
    queue_thr = pthread_create(&queue_id, NULL, 
            &queue_wait, (void *) que);

    pool = packet_pool_init(); 
    reord_args = (cb_reord_args_t *) malloc(sizeof(cb_reord_args_t*));
    reord_args->pool = pool;
    reord_args->queue = que;
    pool_thr = pthread_create(&pool_id, NULL,
            &nudge_queue, (void *) reord_args);

    ind_array = generrate_index(MAX_PACKET);
    encaps_packet_t **packets = NULL;
    packets = (encaps_packet_t **) 
        malloc(sizeof(encaps_packet_t*) * MAX_PACKET);

    encaps_packet_t *temp = NULL;
    for (i = 0; i < MAX_PACKET; i++) {
        temp = (encaps_packet_t *) malloc(sizeof(encaps_packet_t*));
        fill_packet(ind_array[i], temp);
        printf("seq: %d, raw: %s \n", 
                ind_array[i], temp->raw_packet);
        packets[i] = temp;
    }

    for (i = 0; i < MAX_PACKET; i++) {
        pthread_mutex_lock(&pool->lock);
        push2pool((char *) packets[i], pool);
        sleep(1);
    }
    
    diff = clock() - start;
    msec = diff * 1000 / CLOCKS_PER_SEC; 
    while (msec < RUNTIME) {
        diff = clock() - start;
        msec = diff * 1000 / CLOCKS_PER_SEC;
    }
    /*printf("**************\n");*/
    /*printf("killing threads bitches\n");*/
    /*pthread_kill(dest_id, SIGKILL);*/
    /*pthread_kill(queue_id, SIGKILL);*/
    /*pthread_kill(pool_id, SIGKILL);*/
    /*printf("**************\n");*/

    pthread_join(dest_id, NULL);
    pthread_join(queue_id, NULL);
    pthread_join(pool_id, NULL);
}



/**
 * @brief 
 *
 * Checks whether integer exist in integer
 * array
 *
 * @param[in] val
 * @param[in] arr
 * @param[in] size
 *
 * @return 
 */
static bool isvalue_inarray(int val, 
        int *arr, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (arr[i] == val) {
            return true;
        }
    }
    return false;
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
static int* generrate_index(int size) {
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
 * @brief 
 *
 * Generate bursty encapsulated packet.
 *
 * @param[in] seq_number
 * @param[out] packet
 */
static void fill_packet(int seq_number, 
        encaps_packet_t *packet) {
    int i = 0;
    char randomletter = 'A' + (rand() % 26);
    packet->seq = seq_number;
    for (i = 0; i < BLOCKSIZE; i++) {
        packet->raw_packet[i] = randomletter;
    }
}

/**
 * @brief 
 *
 * This function tests queue thread. ,
 * First it creates destination thread 
 * with init_receive()
 * Followingly, function starts queue thread 
 * with queue_wait(), 
 * fills pointer array of queue with packets, then 
 * signals the queue.
 * Consequently, queue forwards 
 * data to destination socket.
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 */
static void execute_queue(char *dest_ip, char *dest_port) {
    int dest_thr, queue_thr, test_thr;
    pthread_t dest_id, queue_id, test_id;
    queue_t* que = NULL;

    dest_thr = pthread_create(&dest_id, NULL, &init_receive,
            dest_port);

    sleep(5);

    que = queue_init(dest_ip, dest_port);

    if (dest_thr < 0) {
        perror("could not initiate receite thread");
    }

    sleep(5);

    // initialize queue thread, which would wait packets to come
    queue_thr = pthread_create(&queue_id, NULL, 
            &queue_wait, (void *) que);
    printf("queue_thr: %d\n", queue_thr);

    sleep(5); 
    
    test_thr = pthread_create(&test_id, NULL, 
            &test_queue, (void *) que);
    printf("test_thr: %d\n", test_thr);

    pthread_join(dest_id, NULL);
    pthread_join(queue_id, NULL);
    pthread_join(test_id, NULL);
}

/**
 * @brief 
 *
 * Queue test thread, adds random amount of
 * data to pointer array of queue and wakes
 * up queue
 *
 * @param args
 *
 * @return 
 */
static void *test_queue(void *args) {

    int count, i = 0;
    int index = 0;
    char *msg;

    queue_t *que = (queue_t *) args;
    msg = (char *) malloc(BLOCKSIZE);
    strcpy(msg, "osmank");
    printf("msg: %s\n", msg);

    while (1) {
        pthread_mutex_lock(&que->lock);
        index = 0;
        count = ceil(rand() % MAX_PACKET);
        printf("=====================\n");
        printf("==count: %d\n", count);
        for (i = 0; i <= count; i++) {
            index = que->index + i;
            que->buffer[index] = msg;
        } 
        que->index = index;

        printf("==index: %d\n", index);


        printf("== sending signal \n");
        sleep(1);
        pthread_mutex_unlock(&que->lock);
        pthread_cond_signal(&que->cond);
        printf("== signal sent == \n");
        printf("== test unlock signal \n");
        printf("=====================\n");
    }
    return NULL;
}

/**
 * @brief 
 *
 * end destination emulate thread, waits
 * packets from socket file descriptor
 * of queue. Consequently, writes into file 
 *
 * @param args
 *
 * @return 
 */
static void *init_receive(void *args) {

    printf("in init_receive thread \n");

    char *dest_port = NULL;
    FILE *fp = NULL;

    int rs_addr;
    int yes = 1;
    int sockfd;
    int set_val, bind_val, listen_val;
    struct debug_receiver_args *rx_args = NULL;

    unsigned char *raw_buf = 
        (unsigned char *) malloc(BLOCKSIZE);
    struct addrinfo hints, *addr, *servinfo;
    struct sockaddr_storage their_addr;

    // set hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // get addrinfo of this machine
    
    rx_args = (struct debug_receiver_args *) args;
    dest_port = rx_args->dest_port;
    fp = fopen(rx_args->filename, "w");
    rs_addr = getaddrinfo(NULL, dest_port, 
            &hints, &servinfo);

    if (rs_addr != 0) {
        fprintf(stderr, 
                "getaddrinfo: %s\n", gai_strerror(rs_addr));
    }
    addr = servinfo;
    free(servinfo);

    // initialize socket
    sockfd = socket(addr->ai_family, 
            addr->ai_socktype, addr->ai_protocol);
    set_val = 
        setsockopt(sockfd, SOL_SOCKET, 
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

    socklen_t sin_size;
    sin_size = sizeof(their_addr);

    printf("end destination listens \n");
    listen_val = listen(sockfd, 1);
    printf("listen_val: %d\n", listen_val);

    printf("end destination waits connection \n");
    sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    printf("end destination accepted \n");

    int numbytes = 0;
    unsigned long recv_count = 0;

    printf("end destination ready to get data \n");
    while(1) {
        /*printf("in receive loop \n");*/
        /*printf("recv_count: %d\n", (int) recv_count);*/
        /*printf("BLOCKSIZE: %d\n", (int) BLOCKSIZE);*/
        while (recv_count < BLOCKSIZE) {
            printf("before read \n");
            numbytes = read(sockfd, raw_buf, BLOCKSIZE); 
            /*printf("numbytes: %d\n", numbytes);*/
            if (numbytes > 0) {
                recv_count += numbytes;
            }
        }
        /*printf("END DESTINATION RECV_COUNT: %ld\n", recv_count);*/
        printf("END DESTINATION RECEIVED: %s\n", raw_buf);
        fp = fopen(rx_args->filename, "a");
        fprintf(fp, "%s", raw_buf);
        fprintf(fp, "%s", "\0");
        fclose(fp);
        recv_count = 0;
        numbytes = 0;
        free(raw_buf);
        raw_buf = (unsigned char *) malloc(BLOCKSIZE);
    }
}








