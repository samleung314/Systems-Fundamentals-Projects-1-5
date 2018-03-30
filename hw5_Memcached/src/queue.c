#include "queue.h"
#include "errno.h"
#include "debug.h"

void Sem_wait(sem_t *sem){
try_sem_wait:
    if(sem_wait(sem) < 0){
        if(errno == EINTR){
            goto try_sem_wait;
        }
    }
}

queue_t *create_queue(void) {
    queue_t* queue = calloc(1, sizeof(queue_t));

    int semVal = sem_init(&(queue->items), 0, 0); //initalize semaphore to 0; (0)shared between threads
    int mutexVal = pthread_mutex_init(&(queue->lock), NULL); //zero on success

    if(semVal < 0 || queue == NULL || mutexVal) return NULL;

    queue->front = queue->rear = 0; //intially, queue is empty
    queue->invalid = false;

    return queue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    pthread_mutex_lock(&(self->lock));
    if((self == NULL) || (destroy_function == NULL) || (self->invalid)){
        errno = EINVAL;
        pthread_mutex_unlock(&(self->lock));
        return false;
    }

    queue_node_t* p = self->front;
    while(p != self->rear){
        destroy_function(p->item);

        queue_node_t* freeP = p;
        p = p->next;
        free(freeP);
    }

    if(p == self->rear){
        destroy_function(p->item);
        free(p);
    }

    self->invalid = true;
    pthread_mutex_unlock(&(self->lock));

    return true;
}

bool enqueue(queue_t *self, void *item) {
    pthread_mutex_lock(&(self->lock));
    if((self == NULL) || (self->invalid)){
        errno = EINVAL;
        pthread_mutex_unlock(&(self->lock));
        return false;
    }

    queue_node_t* node = calloc(1, sizeof(queue_node_t));
    node->item = item;

    if(self->front == NULL){
        self->front = self->rear = node;
    }
    else if(self->front == self->rear){  //one element
        self->front->next = node;
        self->rear = node;
    }else{ //add item to back of queue
        self->rear->next = node;
        self->rear = node;
    }

    sem_post(&(self->items));
    pthread_mutex_unlock(&(self->lock));
    return true;
}

void *dequeue(queue_t *self) {
    //sem_wait(&(self->items));
    Sem_wait(&(self->items));
    if((self == NULL) || (self->invalid)){
        errno = EINVAL;
        sem_post(&(self->items));
        return NULL;
    }

    pthread_mutex_lock(&(self->lock));

    int semVal = 0;
    sem_getvalue(&(self->items), &semVal);

    queue_node_t* remove = self->front;
    void* item = remove->item;

    if(semVal==0) self->front = self->rear = remove->next;
    else self->front = remove->next;

    free(remove);

    pthread_mutex_unlock(&(self->lock));
    return item;
}