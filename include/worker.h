#ifndef WORKER_H_
#define  WORKER_H_

#include <sys/types.h>
#include <sys/epoll.h>

#define MAX_EVENTS 4096
#define READ_BUFFER_MAX 4096

typedef struct {
    int socketfd;
    int epollfd;
    pid_t process_id;
    struct epoll_event *events;

} worker_t;




int worker_init(worker_t *worker,int socket_fs);
void worker_run(worker_t *worker);
#endif