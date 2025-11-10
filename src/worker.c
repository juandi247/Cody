// #include <include/worker.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h> // Necesario para usar malloc y free
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <worker.h>
#include <http.h>
#include <string.h>

void parser_init(http_parser_t *parser) {
    parser->state = START_PARSING; 
    parser->currPosition = 0;
    parser->startValuePosition = 0;
}
void request_init(http_request_t *req) {
    memset(req, 0, sizeof(http_request_t));
}


int worker_handle_accept(worker_t *worker, int client_fs) {
  int opt = 1;

  if (setsockopt(client_fs, opt, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    perror("erorr seting handle accpet on SO_REUSEADDR");
    return -1;
  }

  struct epoll_event ev;

  ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
  ev.data.fd = client_fs;

  // save it into epoll but this case we save the OTHER SOCKET
  if (epoll_ctl(worker->epollfd, EPOLL_CTL_ADD, client_fs, &ev) == -1) {
    perror("ERROR adding a new client_socket to EPOLL");
    return -1;
  }

  printf("Added new connection succesfully to epoll worker: %i \n",
         worker->process_id);
printf("aholl \n");
printf("client count %i \n", worker->clientCount);
client_conn_t *newClient = &worker->clients[worker->clientCount];
printf("alo?=? \n");
if (newClient==NULL){
  printf("ES NULL WTF");
}
printf("clientCount=%i MAX_CONNECTIONS=%d\n", worker->clientCount, MAX_CONNECTIONS);
newClient->fd= client_fs;
newClient->buffer= malloc(INITIAL_BUFFER_SIZE);
  newClient->bufCurrSize=0;
  newClient->bufCapacity= INITIAL_BUFFER_SIZE;
  parser_init(&newClient->parser);
request_init(&newClient->client_request);
worker->clientCount++;
printf("Terminamos! volvemos al epoll \n");
  return 0;
}




int worker_init(worker_t *worker, int socket_fs) {
  printf("vamos a hacer init de un worker \n");
  memset(worker, 0, sizeof(worker_t));
  worker->socketfd = socket_fs;
  worker->epollfd = epoll_create1(0);
  worker->process_id = getpid();

  if (worker->epollfd == -1) {
    perror("erorr creating epoll on a worker with pid");
    return -1;
  }

  // Register the type of events that go into our epoll and where? on our epollf
  // and from our socketfd

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = worker->socketfd;

  if (epoll_ctl(worker->epollfd, EPOLL_CTL_ADD, worker->socketfd, &ev) == -1) {
    perror("error creating epoll ctl on worker");
    return -1;
  }

  worker->events = malloc(sizeof(struct epoll_event) * MAX_EVENTS);

  if (!worker->events) {
    perror("ERORR allocating memory for the events");
    return -1;
  }

  worker->clients= calloc(MAX_CONNECTIONS, sizeof(client_conn_t));

  if (worker->clients ==NULL){
    perror("ERROR allocating memory on the clients array for the workers");
    return -1;
  }
  return 0;
}

int close_client_connection(worker_t *worker, int client_fs){
  if (epoll_ctl(worker->epollfd, EPOLL_CTL_DEL, client_fs, NULL) == -1)
  {
    perror("error removing from epoll");
    return -1;
  }
  close(client_fs);
  // todo: decrease the client count, should delete from the array the client connection. We need to swap with the last one, and change quickly its new number.
  worker->clientCount--;
  printf("client closed  :) \n");
  return 0;
}

void worker_run(worker_t *worker) {

  // sockaddrin client_addr;
  struct sockaddr_in client_addr;
  while (1) {

    printf("starting cylce again \n");
    int incomingEvents =
        epoll_wait(worker->epollfd, worker->events, MAX_EVENTS, -1);
    if (incomingEvents == -1) {
      printf("ERROR EN EL WAIT \n");
      return;
    }

    // if (incomingEvents==0){
    //  room for optimization, if there is no incoming request we can put some
    //  sleep or something but later!!!
    //          }

    if (incomingEvents <= 0) {
      printf("no recibimos ningun evento");
      continue;
    }


    printf("Recibimos %i eventos \n", incomingEvents);
    for (int i = 0; i < incomingEvents; i++) {
      int file_descriptor = worker->events[i].data.fd;
      int eventType = worker->events[i].events;

      //?NEW CONNECTION
      if (file_descriptor == worker->socketfd && (eventType & EPOLLIN)) {
        socklen_t lengthAddr = sizeof(client_addr);
        int client_socket = accept(
          worker->socketfd, (struct sockaddr *)&client_addr, &lengthAddr);
          if (client_socket == -1) {
            perror("error accept on client socket");
          }
        printf("CONEX: on worker pid: %i \n", worker->process_id);
        worker_handle_accept(worker, client_socket);
      } //? Data from a client Socket 
      else if (eventType & EPOLLIN) {
        printf("Leemos del worker pid: %i \n-", worker->process_id);
    
        //todo: use a hashtable or something to look quickly this. would be easier
        client_conn_t *currClient=NULL;
        for (int i=0; i<worker->clientCount; i++){
          if (worker->clients[i].fd==file_descriptor){
            currClient= &worker->clients[i];
            printf("encontrmos el client! %i \n", i);
          }else{
            printf("ERROR no encoentramos el CLIENT !!!!! \n");
          }
        }


printf("la capacity deberia ser de  %lu \n", currClient->bufCapacity);
        int nbytes = recv(file_descriptor, currClient->buffer, currClient->bufCapacity, 0);
        printf("leimos??? %i", nbytes);
        
        if (nbytes == -1) {
          printf("--Error -1 en el recv--- \n");
          continue;
        }
        if (nbytes == 0 ) {
          printf("----- No llegaron bytes en el recv ------ \n");
         close_client_connection(worker, file_descriptor);
          continue;
        }
        
        for (int i = 0; i < nbytes; i++) {
         printf("%c", currClient->buffer[i]);
       }
       printf(" \n");
  currClient->bufCurrSize= nbytes + currClient->bufCurrSize;
  parse_init(&currClient->parser, &currClient->client_request);
  parse_http_request(&currClient->parser, currClient->buffer, currClient->bufCurrSize);

      } else if (eventType & EPOLLRDHUP) {
        printf("LLego un hang up, cerramos el socket\n");
        close_client_connection(worker, file_descriptor);
      } else {
        printf("LLEGO uno random?? \n");
      }
    }
  }
}


