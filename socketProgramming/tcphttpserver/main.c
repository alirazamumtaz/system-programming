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


void send_response(struct Request const *req, int client_socket){
    int rv = 0, status = 0;
    struct stat st;

    lstat(req->url,&st);
    time_t now;
    time(&now);

    if(access(req->url,F_OK) != 0)  status = 404;
    else    status = 200;

    
    char *header = (char*) malloc(MAXGETRESPONSESIZE*sizeof(char*));
    char *body   = (char*) malloc(MAXGETRESPONSESIZE*sizeof(char*));//[MAXGETRESPONSESIZE];
    char *buff   = (char*) malloc(MAXGETRESPONSESIZE*sizeof(char*));//[MAXGETRESPONSESIZE];
    
    //Writing Header to the data socket
    rv = snprintf(header, MAXGETREQUESTSIZE, "%s %d OK\nDate:%s\nServer:Ali's Web Server\nContent-Type:text/html\n\n", req->version,status,strtok(ctime(&now),"\n"));
    write(client_socket,header,rv);
    // Writing html/body to the data socket
    rv = sprintf(body,"<!DOCTYPE html><html><head><title>%s</title></head><body><p>",req->url+1);
    write(client_socket,body,rv);
        // fprintf(stderr,"html =%s\n",html);

    if(status == 404){
        rv = sprintf(buff,"\nError 404 - Page Not Found.");
    }
    else if(S_ISDIR(st.st_mode)){
        struct dirent **namelist;
        int n;
        n = scandir(req->url, &namelist, NULL, alphasort);
        for (int i = 0; i < n; i++){
            rv += sprintf(buff,"\n%s%s<br>\n",buff, namelist[i]->d_name);
        }
        write(client_socket,buff,rv);

    }
    else if(S_ISREG(st.st_mode)){
        if(strstr(req->url,".cgi")){
            char cmd[16];
            sprintf(cmd,"/bin/bash %s",req->url);
            int old_std_out = dup(1);
            dup2(client_socket,1);
            system(cmd);
            dup2(1,old_std_out);
        }
        else{
            int fd = open(req->url,O_RDONLY);
            if(fd < 0){
                fprintf(stderr,"File error\n");
                return; 
            }
            char ch[1024];
            int n;
            while((n = read(fd,ch,1024)) != 0){
                write(client_socket,ch,n); // writing byte by byte
            }
            close(fd);
        }
    }
    else {
        fprintf(stderr,"something else");
    }
    
    rv += sprintf(body,"\n</p></body></html>");
    free(header);
    free(body);
    free(buff);
    write(client_socket,body,rv);
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
        send_response(req,data_socket);
            
        close(data_socket);
    }
    

    return 0;
}