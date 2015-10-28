/*
    C ECHO client example using sockets
*/
#include "client_headers.h"
#include "client.h"

int main(int argc , char *argv[])
{
    /*int sock;*/
    /*struct sockaddr_in server;*/
    /*char message[1000] , server_reply[2000];*/
    
    if(argc != 4){
        printf("wrong usage\n");
        printf("%s ip_addr port file_name\n", argv[0]);
        printf("ex: %s 192.168.2.1 5050 kivanc.txt\n", argv[0]);
        exit(EXIT_SUCCESS);
    }else{
        char * ip_addr;
        ip_addr = argv[1];
        char * port;
        port = argv[2];
        char *fname;
        fname = argv[3];
        FILE *fp;
        fp = fopen(fname, "a+");
        execute(ip_addr, port, fp); 
    }
    return 0;
}

void execute(char * ip_addr, char * port, FILE *fp){
    int start_byte = 0;
    int file_size;
    int total_sent = 0;
    int result = 0;
    file_size = get_file_size(fp);
    printf("file_size: %d\n", file_size);
    int block_number = 1;
    char *buffer = (char *) malloc(block_number * BLOCKSIZE);
    memset(buffer, '\0', block_number * BLOCKSIZE);

    //Create socket
    int sock;
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }

    struct sockaddr_in server;
    puts("Socket created");
    server.sin_addr.s_addr = inet_addr(ip_addr);
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(port) );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        exit(EXIT_SUCCESS);
    }

    while (total_sent < file_size){
        read_file(fp, start_byte, block_number, buffer);
        result = send(sock, buffer, block_number * BLOCKSIZE, 0);
        if(result < 0){
            puts("Send failed");
            exit(1);
        }
        printf("%s", buffer);
        total_sent += result;
        start_byte += result;
        memset(buffer, '\0', block_number * BLOCKSIZE);
    }
    free(buffer);
    close(sock);
}
