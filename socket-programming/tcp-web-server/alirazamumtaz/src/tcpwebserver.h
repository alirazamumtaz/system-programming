#ifndef TCPWEBSERVER_H
#define TCPWEBSERVER_H


#define BACKLOG 10
#define MAXGETREQUESTSIZE 2048
#define MAXGETRESPONSESIZE 2048

void reaper(int signum);

typedef struct Request {
    char *url;
    char *version;
} Request;

struct Request *parse_request(const char *raw);
void free_request(struct Request *req);
void send_response(struct Request const *req, int client_socket);
char* str_append(char* dest, char *src);

#endif