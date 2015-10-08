/*
    C socket server example
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    char true_response[10] = "true";
    char unknown_response[10] = "unknown";
    const char sniffer_value[] = "sniffer";
    int str_cmp;
    int shift = 0;
    while(1){

        //accept connection from an incoming client
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock < 0)
        {
            perror("accept failed");
            return 1;
        }
        puts("Connection accepted");
     
        //Receive a message from client
        read_size = recv(client_sock , client_message , 2000 , 0);
        while(read_size > 0 )
        {
            //Send the message back to client
            str_cmp = strcmp(sniffer_value, client_message);
            printf("str_cmp: %d\n", str_cmp);
            printf("client_message: %s\n", client_message);
            printf("sniffer_value: %s\n", sniffer_value);
            if(strcmp(sniffer_value, client_message) == 0){                
                write(client_sock , true_response , strlen(true_response));
                read_size = 0;
            }else{
                write(client_sock , unknown_response , strlen(unknown_response));
                read_size = 0;
            }
            bzero(client_message, 2000);
        }                 
        if(read_size == 0)
        {
            /*puts("Client disconnected");*/
            /*fflush(stdout);*/
        }
        else if(read_size == -1)
        {
            perror("recv failed");
        }
    }         
    return 0;
}
