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
    int block_number = 2;
    char *buffer = (char *) malloc(block_number * BLOCKSIZE);
    memset(buffer, '\0', block_number * BLOCKSIZE);
    read_file(fp, start_byte, block_number, buffer);
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    
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
     
    /*puts("Connected\n");*/

    int result = 0;
    result = send(sock, buffer, strlen(buffer), 0);
    if(result < 0){
        puts("Send failed");
        exit(EXIT_SUCCESS);
        /*return 1;*/
    }else{
        printf("%d \n", result);
    }

    result = recv(sock, server_reply, 2000, 0);
    if(result < 0){
        puts("Receive failed");
        exit(EXIT_SUCCESS);
        /*return 1;*/
    }

    puts(server_reply);
    close(sock);
}
