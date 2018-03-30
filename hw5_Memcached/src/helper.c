#include "helper.h"

bool helpMenu(int argc, char *argv[]){

    for(int i=0; i<argc; i++){
        if(!strcmp(argv[i], "-h")){
            printf(HELPTEXT);
            return true;
        }
    }
    return false;
}

bool validArgs(int argc, char *argv[]){
    if(argc != 4) return false;
    int NUM_WORKERS = strtol(argv[1], NULL, 10);
    int PORT_NUMBER = strtol(argv[2], NULL, 10);
    int MAX_ENTRIES = strtol(argv[3], NULL, 10);

    return (NUM_WORKERS >= 1) && (PORT_NUMBER >= 0) && (MAX_ENTRIES >= 1);
}

int open_listenfd(char *port){
    struct addrinfo hints, *listp, *p;
    int listenfd, optval=1;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number */
    getaddrinfo(NULL, port, &hints, &listp);


    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;   //Socket failed, try the next

        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        close(listenfd); /* Bind failed, try the next */

    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 1024) < 0) {
        close(listenfd);
    return -1;
    }
    return listenfd;
}

void handleRequest(int connfd, hashmap_t *hashmap){
    request_header_t header;
    Read(connfd, &header, sizeof(request_header_t));

    void *key = calloc(1, header.key_size);
    void *value = calloc(1, header.value_size);
    Read(connfd, key, header.key_size);
    Read(connfd, value, header.value_size);

    map_key_t mapKey = MAP_KEY(key, header.key_size);
    map_val_t mapVal = MAP_VAL(value, header.value_size);

    bool validKey = (header.key_size >= MIN_KEY_SIZE) && (header.key_size <= MAX_KEY_SIZE);
    bool validValue = (header.value_size >= MIN_VALUE_SIZE) && (header.value_size <= MAX_VALUE_SIZE);

    switch(header.request_code){
        case PUT:
            if(!validKey || !validValue){
                badSize(connfd);
                break;
            }
            debug("PUT");
            putRequest(connfd, mapKey, mapVal, hashmap);
            break;

        case GET:
            if(!validKey){
                badSize(connfd);
                break;
            }
            debug("GET");
            getRequest(connfd, mapKey, hashmap);
            break;

        case EVICT:
            if(!validKey){
                    badSize(connfd);
                    break;
            }
            debug("EVICT");
            evictRequest(connfd, mapKey, hashmap);
            break;

        case CLEAR:
            debug("CLEAR");
            clearRequest(connfd, hashmap);
            break;

        default:
            debug("Invalid Request");
            invalidRequest(connfd);
            break;
    }
}

void putRequest(int connfd, map_key_t mapKey, map_val_t mapVal, hashmap_t *hashmap){

    bool success = put(hashmap, mapKey, mapVal, true);

    response_header_t response;

    if(success){
        response.response_code = OK;
    }else{
        response.response_code = BAD_REQUEST;
    }

    response.value_size = 0;
    Write(connfd, &response, sizeof(response_header_t));
}

void getRequest(int connfd, map_key_t mapKey, hashmap_t *hashmap){
    map_val_t val = get(hashmap, mapKey);
    response_header_t *response = calloc(1, sizeof(response_header_t));

    if(val.val_base == NULL || val.val_len == 0){
        response->response_code = NOT_FOUND;
        response->value_size = 0;
    }else{
        response->response_code = OK;
        response->value_size = val.val_len;
    }

    Write(connfd, response, sizeof(response_header_t));
    Write(connfd, val.val_base, val.val_len);
    free(response);
}

void evictRequest(int connfd, map_key_t mapKey, hashmap_t *hashmap){
    map_node_t del = delete(hashmap, mapKey);

    destroy_function(del.key, del.val);

    response_header_t response;
    response.response_code = OK;
    response.value_size = 0;
    Write(connfd, &response, sizeof(response_header_t));
}

void clearRequest(int connfd, hashmap_t *hashmap){
    clear_map(hashmap);

    response_header_t *response = calloc(1, sizeof(response_header_t));
    response->response_code = OK;
    response->value_size = 0;
    Write(connfd, response, sizeof(response_header_t));
    free(response);
}

void invalidRequest(int connfd){
    response_header_t response;
    response.response_code = UNSUPPORTED;
    response.value_size = 0;
    Write(connfd, &response, sizeof(response_header_t));
}

void badSize(int connfd){
    response_header_t response;
    response.response_code = BAD_REQUEST;
    response.value_size = 0;
    Write(connfd, &response, sizeof(response_header_t));
}

/* Used in item destruction */
void destroy_function(map_key_t key, map_val_t val) {
    debug("KeyBase: %p", (key.key_base));
    debug("ValBase: %p", (val.val_base));
    free(key.key_base);
    free(val.val_base);
}

void Write(int fd, void *buf, size_t count){
    rio_writen(fd, buf, count);
}

void Read(int fd, void *buf, size_t count){
    rio_readn(fd, buf, count);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR){ /* Interrupted by sig handler return */
                nwritten = 0;   /* and call write() again */
            }else if(errno == EPIPE){
                close(fd);
            }else{
                return -1; /* errno set by write() */
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

ssize_t rio_readn(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR) /* Interrupted by sig handler return */
                nread = 0;      /* and call read() again */
            else
                return -1; /* errno set by read() */
        } else if (nread == 0)
            break; /* EOF */
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft); /* return >= 0 */
}