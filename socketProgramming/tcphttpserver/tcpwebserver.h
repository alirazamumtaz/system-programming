#ifndef TCPWEBSERVER_H
#define TCPWEBSERVER_H

typedef struct Request {
    char *url;
    char *version;
} Request;

struct Request *parse_request(const char *raw);
void free_request(struct Request *req);

#endif