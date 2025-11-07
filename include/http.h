#ifndef HTTP_H
#define HTTP_H


#define MAX_METHOD_SIZE 16
#define MAX_URI_SIZE 2048
#define MAX_HEADERS 32
#define MAX_HEADER_SIZE 16
#define MAX_REQUEST_SIZE 64*1024

typedef struct{
    char method[MAX_METHOD_SIZE];
    char URI[MAX_URI_SIZE];
    char version[MAX_METHOD_SIZE];
    char headers[MAX_HEADERS][2][MAX_HEADER_SIZE]; //this is an array of 2 dimensions. We define the maxheaders as the Y, and X as 2.
    // we are going to malloc the body (because most of the request are going to be a get so yea)
    char *body;
    int keepAlive;
}http_request_t;



#endif
