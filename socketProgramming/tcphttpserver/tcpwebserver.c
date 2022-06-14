#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tcpwebserver.h"


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
    req->url = malloc(url_len + 1);
    if (!req->url) {
        free_request(req);
        return NULL;
    }
    memcpy(req->url, raw, url_len);
    req->url[url_len] = '\0';
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