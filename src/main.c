#include <stdio.h>
#include <sys/socket.h>
#include <master.h>
int main(){

    //todo: read from the server.config file instead of hardcoded values

    
    printf("starting server .... \n");

    int port = 8080;
    int workerNumber= 4;
    master_t master;


    if (master_init(&master, port, workerNumber)==-1){
        return;
    }

    master_run(&master);

    return 0;
}
