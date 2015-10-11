/*
    C ECHO client example using sockets
*/
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<stdlib.h>
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr

void execute(char *, char *);
void write_some();
void read_some();

int main(int argc , char *argv[])
{
    /*int sock;*/
    /*struct sockaddr_in server;*/
    /*char message[1000] , server_reply[2000];*/
    
    if(argc != 3){
        printf("wrong usage\n");
        printf("%s ip_addr port\n", argv[0]);
        printf("ex: %s 192.168.2.1 5050\n", argv[0]);
        exit(EXIT_SUCCESS);
    }else{
        char * ip_addr;
        ip_addr = argv[1];
        char * port;
        port = argv[2];
        execute(ip_addr, port); 
    }
    /*read_some();*/
    return 0;
}

void execute(char * ip_addr, char * port){
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    
    /*puts("Socket created");*/
     
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
    /*strcpy(message, argv[3]);*/
    strcpy(message, "hello");
    result = send(sock, message, strlen(message), 0);
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

void write_some(){
    FILE *fp;
    fp = fopen("some.txt", "w+");
    fprintf(fp, "hello fprintf ... \n");
    fputs("shit \n", fp);
    fclose(fp);
}

void read_some(){
    FILE * fp;
    char buff[255];

    fp = fopen("some.txt", "r");
    fscanf(fp, "%s", buff);
    printf("1: %s\n", buff);

    fscanf(fp, "%s", buff);
    printf("2: %s\n", buff);

    fscanf(fp, "%s", buff);
    printf("3: %s\n", buff); 

    fscanf(fp, "%s", buff);
    printf("4: %s\n", buff); 

    fclose(fp);
}

