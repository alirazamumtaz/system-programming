
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "tcpwebserver.h"

void reaper(int signum){
    waitpid(-1, NULL, 0);
}

struct Request *parse_request(const char *raw) {
    struct Request *req = NULL;

    // Dynamically allocating space to request struct
    req = malloc(sizeof(struct Request));
    if (!req) {
        return NULL;
    }
    memset(req, 0, sizeof(struct Request));

    //** Checking the method

    size_t meth_len = strcspn(raw, " "); // this will return position of the fist space - 1
    if (memcmp(raw, "GET", strlen("GET")) != 0) {
        fprintf(stderr,"wrong method");
        exit(1);
    }
    raw += meth_len + 1; // move past space

    // Request-URI
    size_t url_len = strcspn(raw, " ");
    req->url = malloc(url_len + 2);
    if (!req->url) {
        free_request(req);
        return NULL;
    }
    char buff[url_len+1];
    memcpy(buff, raw, url_len);
    buff[url_len] = '\0';
    sprintf(req->url,".%s",buff);
    req->url[url_len+1] = '\0';
    raw += url_len + 1; // move past <SP>

    // HTTP-Version
    size_t ver_len = strcspn(raw, "\r\n");
    req->version = malloc(ver_len + 1);
    if (!req->version) {
        free_request(req);
        return NULL;
    }
    memcpy(req->version, raw, ver_len);
    req->version[ver_len] = '\0';
    return req;
}

void free_request(struct Request *req) {
    free(req->url);
    free(req->version);
    free(req);
}

char* str_append(char* dest, char *src){
    int newsize = strlen(dest) + strlen(src) + 1;
    dest = (char*)realloc(dest,newsize*sizeof(char));
    dest = strcat(dest,src);
    return dest;
}

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
    char buff[64] = {0};
    
    rv = snprintf(header, MAXGETREQUESTSIZE, "%s %d OK\nDate:%s\nServer:Ali's Web Server\nContent-Type:text/html\n\n", req->version,status,strtok(ctime(&now),"\n"));
    write(client_socket,header,strlen(header));
    
    int temprv = 0;
    if(status == 404){
        body = str_append(body,"Error 404 - Page Not Found.");
        write(client_socket,body,strlen(body));

    }

    else if(S_ISDIR(st.st_mode)){
        struct dirent **namelist;
        int n;
        n = scandir(req->url, &namelist, NULL, alphasort);
        fprintf(stderr,"n = %d\n",n);

        for (int i = 0; i < n; i++){
            rv = sprintf(buff,"<br>%s\n",namelist[i]->d_name);
            body = str_append(body,buff);            
        }
        write(client_socket,body,strlen(body));

    }
    else if(S_ISREG(st.st_mode)){
        if(strstr(req->url,".cgi")){
            char cmd[16];
            sprintf(cmd,"/bin/bash %s",req->url);
            int old_std_out = dup(1);
            dup2(client_socket,1);
            system(cmd);
            fprintf(stderr,"Done with it\n");
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
    free(header);
    free(body);
}
