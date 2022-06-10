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

#define BACKLOG 10

int main(int argc, char** argv){
    if(argc != 3){
        perror("arguments not provided.\n");
        exit(1);
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
        
        
        char *raw_request;
        int rv, count = 0;
        char buf[100];
        while (rv = read(data_socket, buf, sizeof(buf))){
            fprintf(stderr,"\nInside loop\n");
            if(count == 0)  sprintf(raw_request,buf);
            else sprintf(raw_request,"%s%s",raw_request,buf);
            count += rv;
        }
        // raw_request[count-1] = NULL;
        fprintf(stderr,"Request is\n\n%s",raw_request);
        // char * response = get_response(request);

        // char* response;
        // int pos = strcspn(raw_request, "\n");
            
        close(data_socket);
    }
    

    return 0;
}