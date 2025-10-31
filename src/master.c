// #include <include/master.h>
#include <master.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <stdlib.h>
// #include <include/worker.h>
#include <worker.h>

int configure_socket_options(int socket_fs){
    int opt=1;
    if (setsockopt(socket_fs, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
        perror("error setting so reuseaddr");
        return -1;
    };
    return 0;
}



int master_init(master_t *master, int port, int workers_num){
    if (workers_num > MAXWORKERS || workers_num <1){
        printf("ERROR - Workers number invalid \n");
        return -1;
    }    

    //Create socket and initial config struct
    master->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (master->socket_fd ==-1){
        perror("ERROR CREATING SOCKET \n");
        return -1;
    }    
    master->port= port;
    master->workers_num= workers_num;



    //set options (posibly new or more on the function)
    if (configure_socket_options(master->socket_fd)==-1){
        printf("error configurando \n");
        return -1;
    }    

    struct sockaddr_in server_addr;
    server_addr.sin_family= AF_INET;
    server_addr.sin_port= htons(master->port);
    server_addr.sin_addr.s_addr= INADDR_ANY;

    //bind 
    if (bind(master->socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))==-1){
        perror("error on bind");
        return -1;
    }    

    //LISTEN
    //todo: check SOMAXCONN for testing purposes!!
    if (listen(master->socket_fd, SOMAXCONN)==-1){
        perror("error on listen!!");
        return -1;
    }    
    return 0;

}    



pid_t fork_worker(master_t *master){
    pid_t procesId= fork();
    if(procesId==-1){
        perror("Fallo crear el worker");
        return -1;
    }else if (procesId==0)
    {
        worker_t new_worker;
        if (worker_init(&new_worker, master->socket_fd)==-1){
            perror("error init worker");
            return -1;
        }
        printf("we created a worker succesfully, starting now with pid: %i \n", getpid());
        worker_run(&new_worker);

        exit(0);
    }
    
    return procesId;
}




void master_run(master_t *master){
    for (int i=0; i<master->workers_num; i++){
        fork_worker(master);
    }


    while (1)
    {
    sleep(10);
    printf("--heartbeat parent-- \n ");
    }
    
}
