#include "client.h"


static void stream_file(char *ip_addr, char *port, 
        int file_size, int conn_number);
static int thr_loop_tasks(char *thr_buff, int *thr_seq_num);
static void *tx_chain(void *args);

int SEQ_NUMBER = 0;
int TX_BYTE_COUNT = 0;
int FILE_SIZE = 0;
FILE *FP;
pthread_mutex_t LOCK;

int main(int argc , char *argv[])
{

    if(argc != 5){
        printf("wrong usage\n");
        printf("%s ip_addr port file_name conn_number\n", argv[0]);
        printf("ex: %s 192.168.2.1 5050 kivanc.txt 3\n", argv[0]);
        exit(EXIT_SUCCESS);
    }else{
        int conn_number;
        char *ip_addr, *port, *fname;

        ip_addr = argv[1];
        port = argv[2];
        fname = argv[3];

        FP = fopen(fname, "a+");

        FILE_SIZE = get_file_size(FP);
        printf("file_size: %d\n", FILE_SIZE);

        printf("conn_number: %s\n", argv[4]);
        conn_number = atoi(argv[4]);

        stream_file(ip_addr, port, FILE_SIZE, conn_number); 
        /*start transmitting*/
    }
    return 0;
}

/**
 * @brief 
 *
 * Threads get portion of file and sequence
 * number from this function.
 *
 * @param[out] thr_buff
 * @param[out] thr_seq_num
 *
 * @return 
 */
static int thr_loop_tasks(char *thr_buff, int *thr_seq_num) {
    pthread_mutex_lock(&LOCK);

    *thr_seq_num = SEQ_NUMBER;
    
    SEQ_NUMBER++;

    read_file(FP, TX_BYTE_COUNT, TX_BYTE_COUNT+BLOCKSIZE, thr_buff); 

    TX_BYTE_COUNT += BLOCKSIZE; 

    pthread_mutex_unlock(&LOCK);

    return 0;
}


/**
 * @brief 
 *
 * @param[in] ip_addr
 * @param[in] port
 * @param[in] file_size
 * @param[in] conn_number
 */
static void stream_file(char *ip_addr, char *port, 
        int file_size, int conn_number){

    printf("file_size: %d\n", file_size);
    int i = 0, conn_res = 0, pthr_res;

    //Create sockets and buffers
    int sockfds[conn_number];
    for(i = 0; i < conn_number; i++) {
        sockfds[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfds[i] == -1) {
            printf("can't create socket %d\n", i);
            exit(0);
        }
    }
    
    pthread_t tids[conn_number];

    //Initialize sockaddr_in struct to connect to dest
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip_addr);
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(port) );

    //Connect socket file descriptors
    cb_tx_args_t cb_args[conn_number];
    for(i = 0; i < conn_number; i++) {
        conn_res = connect(sockfds[i], (struct sockaddr *)&server, 
                sizeof(server));
        if (conn_res < 0) {
            perror("connection failed. Error");
            exit(0);
        }else {
            cb_args[i].sockfd = sockfds[i];
        }
    }

    for(i = 0; i < conn_number; i++) {
        pthr_res = pthread_create(&(tids[i]), NULL, 
                &tx_chain, &(cb_args[i]));
    }
    
    for(i = 0; i < conn_number; i++) {
        pthread_join(tids[i], NULL);
    }
}



/**
 * @brief 
 *
 * Transmitter thread function. Dedicated
 * for single TCP connection.
 *
 * @param args
 *
 * @return 
 */
static void *tx_chain(void *args) {
    
    cb_tx_args_t *cb_args = (cb_tx_args_t *) args;
    int task_res;
    unsigned long temp_count = 0;
    int send_res;
    struct encaps_packet packet;
    int dbg_sent = 2;

    while (1) {
        task_res = thr_loop_tasks(cb_args->buff, &(cb_args->seq_num));

        if (TX_BYTE_COUNT >= FILE_SIZE) {
            printf("FILE TRANSMITTED\n");
            printf("FILE_SIZE: %d\n", FILE_SIZE);
            printf("TX_BYTE_COUNT: %d\n", TX_BYTE_COUNT);
            return NULL;
        }

        if (task_res != 0) {
            break;
        }else {
            memcpy(packet.raw_packet, cb_args->buff, BLOCKSIZE);
            packet.seq = cb_args->seq_num;
            temp_count = 0;
            
            while (temp_count < PACKET_SIZE) {
                send_res = send(cb_args->sockfd, &((unsigned char *) &packet)[temp_count],
                        dbg_sent, 0);
                if (send_res > 0) {
                    temp_count += send_res;
                    sleep(1);
                }
            }
        }
    }
}

