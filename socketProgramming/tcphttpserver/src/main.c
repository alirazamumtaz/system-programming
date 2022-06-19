#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "tcpwebserver.h"



int main(int argc, char** argv){
    signal(SIGCHLD, reaper);
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
        int cpid = fork();
        if(cpid == 0){
            int rv;
            // To read the http request 
            char raw_request[MAXGETREQUESTSIZE];
            memset(raw_request,'\0',MAXGETREQUESTSIZE);
            rv = read(data_socket, raw_request, sizeof(raw_request));

            int n = 0;
            // Parse request to get http version and URL
            Request *req = parse_request(raw_request);
            send_response(req,data_socket);
            free_request(req);
            close(data_socket);
            exit(0);
        }
        close(data_socket);
    }
    return 0;
}