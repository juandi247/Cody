#ifndef MASTER_H_
#define MASTER_H_



#define MAXWORKERS 32
#define DEFAULT_WORKERS 4



typedef struct{
    int socket_fd;
    int port;
    int workers_num;
    // future fields like is shutting down or something idk yet
}master_t;

int master_init(master_t *master, int port, int workerNumber);
int master_run(master_t *master);
#endif //MASTER_H