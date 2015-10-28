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
        execute(ip_addr, port, fname); 
    }
    /*read_some();*/
    return 0;
}

void execute(char * ip_addr, char * port, char * fname){
    FILE *fp;
    fp = fopen(fname, "a+");
    int start_byte = 0;
    int file_size;
    int total_sent = 0;
    int result = 0;
    file_size = fseek(fp, 0L, SEEK_END);/*get file size*/
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET); /* initialize fseek pointer */
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
        /*return 1;*/
    }

    int counter = 0;
    while (total_sent < file_size){
        read_file(fp, start_byte, block_number, buffer);
        /*printf("buffer place: %p\n", buffer);*/
        /*printf("%s", buffer);*/
        /*result = send(sock, buffer, strlen(buffer), 0);*/
        result = send(sock, buffer, block_number * BLOCKSIZE, 0);
        if(result < 0){
            puts("Send failed");
            exit(1);
        }else{
            /*printf("%d \n", result);*/
        }
        printf("%s", buffer);
        /*printf("buffer: %s\n", buffer);*/
        /*printf("result: %d\n", result);*/
        /*printf("total_sent: %d\n", total_sent);*/
        /*printf("result: %d\n", result);*/
        total_sent += result;
        start_byte += result;
        /*printf("total_sent: %d\n", total_sent);*/
        counter += 1;
        /*if (counter == 25){*/
            /*exit(1);*/
        /*}*/
        memset(buffer, '\0', block_number * BLOCKSIZE);
        buffer = buffer - (block_number * BLOCKSIZE);
    }
    close(sock);
}
