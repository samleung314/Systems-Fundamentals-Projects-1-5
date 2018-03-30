#include <stdio.h>
#include <stdlib.h>


#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "helper.h"

queue_t* queue;
hashmap_t* hashmap;

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);

    if(helpMenu(argc, argv)) return EXIT_SUCCESS;
    if(!validArgs(argc, argv)) return EXIT_FAILURE;

    int NUM_WORKERS = strtol(argv[1], NULL, 10);
    char *PORT_NUMBER = argv[2];
    int MAX_ENTRIES = strtol(argv[3], NULL, 10);

    queue = create_queue();
    hashmap = create_map(MAX_ENTRIES, jenkins_one_at_a_time_hash, destroy_function);

    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    int connfd, listenfd = open_listenfd(PORT_NUMBER);
    pthread_t tid;

    //create worker threads
    for(int i=0; i<NUM_WORKERS; i++){
        pthread_create(&tid, NULL, thread, NULL);
    }

    while(true){
        clientlen = sizeof(struct sockaddr_storage);
        struct sockaddr *addr = (struct sockaddr*)&clientaddr;
        connfd = accept(listenfd, addr, &clientlen);
        enqueue(queue, &connfd);
    }

    exit(0);
}

void *thread(void *vargp){
    pthread_detach(pthread_self());
    while(true){
        int connfd = *(int*)dequeue(queue);
        handleRequest(connfd, hashmap);
        close(connfd);
    }
}