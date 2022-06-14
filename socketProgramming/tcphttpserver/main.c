#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "tcpwebserver.h"

#define BACKLOG 10
#define MAXGETREQUESTSIZE 2048

int main(int argc, char** argv){
    if(argc != 3){
        fprintf(stderr,"arguments not provided.\n");
        fprintf(stderr,"Usage: ./a.out <ip_address> <port_number>.\n");
        exit(-1);
    }

    // Creating a socket for listening client requests
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    // This will be used to store client's socket (data socket fro server)
    int data_socket;

    // Two sockadd_in variables for server and client.
    struct sockaddr_in server_address, client_address;

    // Initialising server address in order to bind and then listen for requests
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1],&server_address.sin_addr);
    memset(&(server_address.sin_zero), '\0', sizeof(server_address));

    // Calling bind function in order to bind the socket with server
    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    
    // Start listening on provide port
    listen(server_socket,BACKLOG);


    while (1){
        fprintf(stderr,"\nServer waiting for client connection...");

        int client_addr_length = sizeof(client_address);

        // This call be blocked till a client knocks at the provided port and ip
        data_socket = accept(server_socket,(struct sockaddr*)&client_address,&client_addr_length);
        fprintf(stderr,"\nCONNECTION ESTABLISHED\n");
        
        int rv;
        // To read the http request 
        char raw_request[MAXGETREQUESTSIZE];
        memset(raw_request,'\0',MAXGETREQUESTSIZE);
        rv = read(data_socket, raw_request, sizeof(raw_request));

        Request *req = parse_request(raw_request);
	fprintf(stderr,"%s\n",req->url);
        int response_size = 1024;
        char *response =  (char*)malloc(response_size*sizeof(char));
        memset(response,'\0',response_size);
        char test[4] = "123";
        sprintf(response,"%s 200 OK\nConnection: keep-alive\nContent-Encoding: gzip\nContent-Type: text/html; charset=utf-8\nTransfer-Encoding: chunked\n\n%s",req->version,test);
        //fprintf(stderr,"%s",response);
	int i = 0;
	while(rv=write(data_socket,response+(i++),1));
        //write(data_socket,response,strlen(response));
            
        close(data_socket);
    }
    

    return 0;
}
