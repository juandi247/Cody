#ifndef WORKER_H_
#define  WORKER_H_

#include <sys/types.h>
#include <sys/epoll.h>
// #include <include/http.h>
// #include <include/fsmParser.h>
#include <http.h>
#include <fsmParser.h>


#define MAX_EVENTS 4096
#define INITIAL_BUFFER_SIZE 8012
#define MAX_CONNECTIONS 100000

typedef struct {
    int fd;
    char *buffer;  
    size_t bufCurrSize;
    size_t bufCapacity;
    int keep_alive;  
    // char client_ip[INET_ADDRSTRLEN];
    http_request_t client_request;
    http_parser_t parser;
} client_conn_t;

typedef struct {
    int socketfd;
    int epollfd;
    pid_t process_id;
    struct epoll_event *events;
    client_conn_t *clients;
    int clientCount;
} worker_t;






int worker_init(worker_t *worker,int socket_fs);
void worker_run(worker_t *worker); 

#endif