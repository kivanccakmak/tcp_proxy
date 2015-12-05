#include "stream.h"

static void stream(char *ip_addr, char *port,
        FILE *fp);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Wrong Usage\n");
        printf("%s dest_ip dest_port file_name\n", argv[0]);
        return 0;
    }

    char *ip_addr, *port, *fname;
    FILE *fp;
    
    ip_addr = argv[1];
    port = argv[2];
    fname = argv[3];

    fp = fopen(fname, "a+");
    stream(ip_addr, port, fp);
}

/**
 * @brief 
 *
 * @param[in] ip_addr
 * @param[in] port
 * @param[in] fp
 */
static void stream(char *ip_addr, char *port, FILE *fp)
{ 
    int conn_res = 0, sockfd;
    int byte_count = 0, temp_count = 0, capacity = 0;
    int file_size = 0, amount = 0, residue = 0;
    char *thr_buff;

    //Initialize sockaddr_in struct to connect to dest
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip_addr);
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(port) );

    printf("socket initializing ...\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    printf("connecting ...\n");
    conn_res = connect(sockfd, (struct sockaddr*)&server,
            sizeof(server));
    if (conn_res < 0) {
        perror("connection failed. Error");
    }

    thr_buff = (char *) malloc(BLOCKSIZE);
    file_size = get_file_size(fp);

    while (byte_count < file_size) {
        if (byte_count + BLOCKSIZE < file_size) {
            read_file(fp, byte_count, byte_count + BLOCKSIZE, thr_buff);
            capacity = BLOCKSIZE;
        } else {
            residue = file_size - byte_count;
            read_file(fp, byte_count, byte_count + residue, thr_buff);
            capacity = residue;
        }
        while (temp_count < capacity) {
            amount = write(sockfd, thr_buff+temp_count, 
                    capacity-temp_count); 
            if (amount > 0) {
                temp_count += amount;
            }
        }
        byte_count += temp_count;
        temp_count = 0;
    }
} 
