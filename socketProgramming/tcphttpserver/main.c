#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BACKLOG 10

int main(){
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int data_socket;

    struct sockaddr_in server_address, client_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(54154);
    inet_aton("192.168.64.4",&server_address.sin_addr);
    memset(&(server_address.sin_zero), '\0', sizeof(server_address));
    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    listen(server_socket,BACKLOG);
    while (1){
        fprintf(stderr,"\nServer waiting for client connection...");

        int client_addr_length = sizeof(client_address);
        data_socket = accept(server_socket,(struct sockaddr*)&client_address,&client_addr_length);
        fprintf(stderr,"\n*********  CONNECTION ESTABLISHED   **********\n");
        
        // char * response = get_response(request);

        char buf[100];
        // char* response;
        int rv;
        while (rv = read(data_socket, buf, sizeof(buf)))
            write(1,buf,rv);
        close(data_socket);
    }
    

    return 0;
}