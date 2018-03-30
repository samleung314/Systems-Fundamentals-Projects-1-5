#ifndef HELPER
#define HELPER

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#ifdef EC
#include "extracredit.h"
#else
#include "hashmap.h"
#endif

#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "debug.h"

#define HELPTEXT "./cream [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES\n\
-h                 Displays this help menu and returns EXIT_SUCCESS.\n\
NUM_WORKERS        The number of worker threads used to service requests.\n\
PORT_NUMBER        Port number to listen on for incoming connections.\n\
MAX_ENTRIES        The maximum number of entries that can be stored in `cream`'s underlying data store.\n"

bool helpMenu(int argc, char *argv[]);
bool validArgs(int argc, char *argv[]);
void destroy_function(map_key_t key, map_val_t val);
void *thread(void *vargp);
int open_listenfd(char *port);
void handleRequest(int connfd, hashmap_t *hashmap);

void putRequest(int connfd, map_key_t mapKey, map_val_t mapVal, hashmap_t *hashmap);
void getRequest(int connfd, map_key_t mapKey, hashmap_t *hashmap);
void evictRequest(int connfd, map_key_t mapKey, hashmap_t *hashmap);
void clearRequest(int connfd, hashmap_t *hashmap);
void invalidRequest();

void response(bool success);
bool validKey(uint32_t key_size);
bool validSize(uint32_t value_size);
void badSize(int connfd);

void Write(int fd, void *buf, size_t count);
void Read(int fd, void *buf, size_t count);

ssize_t rio_writen(int fd, void *usrbuf, size_t n);
ssize_t rio_readn(int fd, void *usrbuf, size_t n);

void hello();

#endif