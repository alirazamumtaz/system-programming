#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <netdb.h>
#include <dirent.h>
#include <limits.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "tcpwebserver.h"

#define BACKLOG 10
#define MAXGETREQUESTSIZE 2048
#define MAXGETRESPONSESIZE 2048

int send_response_header(struct Request const *req, int client_socket){
    time_t now;
    time(&now);
    char *response = (char*)malloc(MAXGETRESPONSESIZE*sizeof(char));
    memset(response,'\0',MAXGETRESPONSESIZE);
    int rv = snprintf(response, MAXGETREQUESTSIZE, "%s 200 OK\nDate:%s\nServer:Ali's Web Server\nContent-Type:text/html\n\n", req->version,strtok(ctime(&now),"\n"));
    write(client_socket,response,rv);
    free(response);
    return 200;
}

int send_response_body(char *url, int client_socket){
    int rv = 0;
    struct stat st;
    lstat(url,&st);

    if(S_ISDIR(st.st_mode)){

        char html_start[MAXGETRESPONSESIZE];
        char html_end[100];
        rv = sprintf(html_start,"<!DOCTYPE html><html><head><title>%s</title> </head><body><p>",url);

        struct dirent **namelist;
        int n;
        sprintf(url,".%s",url); // if . is removed then it'll look for the root directory /
        n = scandir(url, &namelist, NULL, alphasort);
        fprintf(stderr,"n =%d\n",n);
        for (int i = 0; i < n; i++){
            rv = sprintf(html_start,"%s%s<br>",html_start, namelist[i]->d_name);
        }
        write(client_socket,html_start,rv);

        rv = sprintf(html_end,"</p></body></html>");
        write(client_socket,html_end,rv);
    }
    else{
        fprintf(stderr,"something else");
    }
    return rv;
}

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

        int n = 0;
        // Parse request to get http version and URL
        Request *req = parse_request(raw_request);

        int status;
        status = send_response_header(req,data_socket);
        if(status == 200)   send_response_body(req->url,data_socket);
            
        close(data_socket);
    }
    

    return 0;
}
