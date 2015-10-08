/*
    C ECHO client example using sockets
*/
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
 
int main(int argc , char *argv[])
{
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
     
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(argv[2]) );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    /*puts("Connected\n");*/

    int result = 0;
    strcpy(message, argv[3]);
    result = send(sock, message, strlen(message), 0);
    if(result < 0){
        puts("Send failed");
        return 1;
    }else{
        printf("%d \n", result);
    }

    result = recv(sock, server_reply, 2000, 0);
    if(result < 0){
        puts("Receive failed");
        return 1;
    }

    puts(server_reply);
     
    close(sock);
    return 0;
}
