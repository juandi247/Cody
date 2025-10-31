// #include <include/worker.h>
#include <worker.h>
#include <netinet/in.h>
#include <stdlib.h> // Necesario para usar malloc y free
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>




int worker_handle_accept(worker_t *worker, int client_fs){
    int opt=1;

    if (setsockopt(client_fs, opt, SO_REUSEADDR, &opt, sizeof(opt))==-1){
        perror("erorr seting handle accpet on SO_REUSEADDR");
        return -1;
    }

    struct epoll_event ev;

    ev.events= EPOLLIN | EPOLLET;
    ev.data.fd= client_fs;

    //save it into epoll but this case we save the OTHER SOCKET
    if (epoll_ctl(worker->epollfd, EPOLL_CTL_ADD, client_fs, &ev)==-1){
        perror("ERROR adding a new client_socket to EPOLL");
        return -1;
    }

    printf("Added new connection succesfully to epoll. \n");

    return 0;
}






int worker_init(worker_t *worker, int socket_fs) {

  worker->socketfd = socket_fs;
  worker->epollfd = epoll_create1(0);
  worker->process_id= getpid();


  if (worker->epollfd == -1) {
    perror("erorr creating epoll on a worker with pid");
    return -1;
  }

  // Register the type of events that go into our epoll and where? on our epollf
  // and from our socketfd

   struct epoll_event ev;
    ev.events= EPOLLIN;
    ev.data.fd= worker->socketfd;

  if (epoll_ctl(worker->epollfd, EPOLL_CTL_ADD, worker->socketfd,&ev) == -1) {
    perror("error creating epoll ctl on worker");
    return -1;
  }

  worker->events = malloc(sizeof(struct epoll_event) * MAX_EVENTS);

  if (!worker->events) {
    perror("ERORR allocating memory for the events");
    return -1;
  }

  return 0;
}




void worker_run(worker_t *worker) {


  // sockaddrin client_addr;
  struct sockaddr_in client_addr;
  while (1) {
    
    printf("starting cylce again \n" );
    int incomingEvents =
        epoll_wait(worker->epollfd, worker->events, MAX_EVENTS, -1);
    if (incomingEvents == -1) {
      printf("ERROR EN EL WAIT \n");
      return;
    }
    


    //         if (incomingEvents==0){
    // room for optimization, if there is no incoming request we can put some
    // sleep or something but later!!!
    //         }

    // means that we received something
    if (incomingEvents > 0) {
        printf("Recibimos %i eventos \n", incomingEvents);
      // go through the queue of events
      for (int i = 0; i < incomingEvents; i++) {
        int file_descriptor = worker->events[i].data.fd;
        int eventType = worker->events[i].events;
        // printf("EVENT TYPE %i \n", eventType);
        //? we received an accept because its on the socketfd from the server
        if (file_descriptor == worker->socketfd && (eventType & EPOLLIN)) {
          printf("NEW CONEX on worker pid: %i \n", worker->process_id);
          socklen_t lengthAddr= sizeof(client_addr);
          int client_socket =accept(worker->socketfd, (struct sockaddr *)&client_addr, &lengthAddr);
          if (client_socket == -1) {
            perror("error accept on client socket");
          }
          worker_handle_accept(worker, client_socket);

        } else if(eventType & EPOLLIN) {
            //? We received READ EVENT FROM A CURRENT SOCKET

            //todo: i think this should be a malloc but later
            char myBuffer[READ_BUFFER_MAX];
            printf("waiting to read someting \n"); 
            //recv instead of read() because its spezialided for sockets and has more options!!! todo: see that more/
            int nbytes= recv(file_descriptor, &myBuffer, READ_BUFFER_MAX,0);

            if(nbytes>0){
            printf("Finished READING the socket \n"); 
            
            for (int i=0; i<nbytes; i++){
            printf("%c", myBuffer[i]);
            if (myBuffer[i]=='\n'){    
                printf("\n Terminamos de leer \n \n");
                break;
            }

        }
            }
          

        }else if (eventType & EPOLLOUT){

            printf("LLEGO un epollout??? \n");
      
        }else{

            printf("LLEGO uno random?? \n");
         
        }
      }
    }
  }
}
