#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "errno.h"
#include "debug.h"
#include "queue.h"

//#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
//#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
//#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

void deleteQ(queue_t *self, map_node_t *n){
    queue_node_t *pointQ = self->front;
    map_node_t *pointNode = (map_node_t*)pointQ->item;

    //delete front node
    bool front = !memcmp(n->key.key_base, pointNode->key.key_base, n->key.key_len);
    if(front){
        self->front = pointQ->next;
        return;
    }

    //delete last node
    pointQ = self->rear;
    pointNode = (map_node_t*)pointQ->item;
    bool rear = !memcmp(n->key.key_base, pointNode->key.key_base, n->key.key_len);

    pointQ = self->front;
    pointNode = (map_node_t*)pointQ->item;

    queue_node_t *prevQ;
    if(rear){
        while(memcmp(pointNode->key.key_base, ((map_node_t*)self->rear->item)->key.key_base, n->key.key_len)){
            prevQ = pointQ;
            pointQ = pointQ->next;
            pointNode = (map_node_t*)pointQ->item;
        }
        self->rear = prevQ;
        return;
    }

    //delete inbetween node
    pointQ = self->front;
    pointNode =  (map_node_t*)pointQ->item;

    while(memcmp(n->key.key_base, pointNode->key.key_base, n->key.key_len)){
        prevQ = pointQ;
        pointQ = pointQ->next;
        pointNode = (map_node_t*)pointQ->item;
    }
    prevQ->next = pointQ->next;
}

void updateQ(queue_t *self, map_node_t *n){
    deleteQ(self, n);
    enqueue(self, n);
}

void clearQ(queue_t *self){
    self->front = NULL;
    self->rear = NULL;
}

void incrementSize(hashmap_t *self){
    pthread_mutex_lock(&(self->fields_lock));
    self->size++;
    pthread_mutex_unlock(&(self->fields_lock));
}

void decrementSize(hashmap_t *self){
    pthread_mutex_lock(&(self->fields_lock));
    self->size--;
    pthread_mutex_unlock(&(self->fields_lock));
}

void incrementRead(hashmap_t *self){
    pthread_mutex_lock(&(self->fields_lock));
    self->num_readers++;
    pthread_mutex_unlock(&(self->fields_lock));
}

void decrementRead(hashmap_t *self){
    pthread_mutex_lock(&(self->fields_lock));
    self->num_readers--;
    pthread_mutex_unlock(&(self->fields_lock));
}

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    if((capacity<=0) || (hash_function==NULL) || (destroy_function==NULL)){
        errno = EINVAL;
        return NULL;
    }

    hashmap_t *hashmap = calloc(1, sizeof(hashmap_t));
    map_node_t *nodes = calloc(capacity, sizeof(map_node_t));
    int writeVal = pthread_mutex_init(&(hashmap->write_lock), NULL);
    int fieldsVal = pthread_mutex_init(&(hashmap->fields_lock), NULL);

    if((hashmap==NULL) || (nodes==NULL) || writeVal || fieldsVal) return NULL;

    hashmap->capacity = capacity;
    hashmap->size = 0;
    hashmap->nodes = nodes;
    hashmap->hash_function = hash_function;
    hashmap->destroy_function = destroy_function;
    hashmap->num_readers = 0;
    hashmap->invalid = false;

    hashmap->q = create_queue();

    return hashmap;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {

    pthread_mutex_lock(&(self->write_lock));
    bool invalid = (key.key_len == 0) || (key.key_base==NULL) ||
                    (val.val_len == 0) || (val.val_base==NULL) ||
                    (self==NULL) || (self->invalid);

    //error case
    if(invalid){
        debug("Invalid");
        pthread_mutex_unlock(&(self->write_lock));
        return false;
    }

    int index = get_index(self, key);
    map_node_t* mapNode = &(self->nodes[index]);

    //update value if key already exists
    int tempIndex = index;
    bool sameKey = false;
    for(int i=0; i<self->capacity; i++){
        tempIndex %= self->capacity;
        mapNode = &(self->nodes[tempIndex]);

        //if we run into tombstone or NULL, does not exist
        bool notFound = (mapNode->key.key_base == NULL) || (mapNode->key.key_len == 0) ||
                        (mapNode->tombstone);

        if(notFound) break;

        //check if same length and same base
        sameKey = (mapNode->key.key_len == key.key_len) &&
                    !memcmp(mapNode->key.key_base, key.key_base, key.key_len);

        if(sameKey){
            debug("Same Key");
            mapNode->val = val;
            pthread_mutex_unlock(&(self->write_lock));
            return true;
        }
        tempIndex++;
    }

    //check for full map and force
    bool full = self->capacity == self->size;

    //error case
    if(full && !force){
        debug("Error case");
        errno = ENOMEM;
        pthread_mutex_unlock(&(self->write_lock));
        return false;
    }

    mapNode = &(self->nodes[index]);

    //overwrite values
    if(full && force){
        debug("Overwrite");
        mapNode = (map_node_t*)self->q->front->item;
        updateQ(self->q, mapNode);
        self->destroy_function(mapNode->key, mapNode->val);
        mapNode->key = key;
        mapNode->val = val;
        pthread_mutex_unlock(&(self->write_lock));
        return true;
    }

    //map not full, linear probe for empty
    tempIndex = index;
    for(int i=0; i<self->capacity; i++){
        tempIndex %= self->capacity;
        mapNode = &(self->nodes[tempIndex]);

        //if we run into tombstone or NULL, does not exist
        bool empty = (mapNode->key.key_base == NULL) || (mapNode->key.key_len == 0) ||
                        (mapNode->tombstone);

        if(empty) break;
        tempIndex++;
    }

    //evict tombstone node
    if(mapNode->tombstone){
        debug("Tombstone");
        //self->destroy_function(mapNode->key, mapNode->val);
        mapNode->tombstone = false;
    }
    mapNode->key = key;
    mapNode->val = val;

    enqueue(self->q, mapNode);

    incrementSize(self);
    debug("Items: %d", self->size);
    pthread_mutex_unlock(&(self->write_lock));

    return true;
}

map_val_t get(hashmap_t *self, map_key_t key) {
    //READER ENTERS
    pthread_mutex_lock(&(self->fields_lock));
    self->num_readers++;
    //first reader holds write lock
    if(self->num_readers==1) pthread_mutex_lock(&(self->write_lock));

    //check for invalid
    bool invalid = (key.key_len == 0) || (key.key_base==NULL) || (self == NULL) || (self->invalid);
    if(invalid){
        errno = EINVAL;
        self->num_readers--;
        pthread_mutex_unlock(&(self->write_lock));
        pthread_mutex_unlock(&(self->fields_lock));
        return MAP_VAL(NULL, 0);
    }

    //allow other readers to enter
    pthread_mutex_unlock(&(self->fields_lock));

    //START CRITICAL SECTION
    int index = get_index(self, key);
    map_node_t* mapNode = &(self->nodes[index]);

    int tempIndex = index;
    bool foundKey = false, notFound = false;
    for(int i=0; i<self->capacity; i++){
        tempIndex %= self->capacity;
        mapNode = &(self->nodes[tempIndex]);

        //if we run into tombstone or NULL, key does not exist
        notFound = (mapNode->key.key_base == NULL) || (mapNode->key.key_len == 0) ||
                        (mapNode->tombstone);

        if(notFound) break;

        //check if same length and same base
        foundKey = (mapNode->key.key_len == key.key_len) &&
                    !memcmp(mapNode->key.key_base, key.key_base, key.key_len);

        if(foundKey) break;
        tempIndex++;
    }

    if(foundKey){

        updateQ(self->q, mapNode);

        //READER LEAVES
        pthread_mutex_lock(&(self->fields_lock));
        self->num_readers--;
        //last reader unlocks writeLock
        if(self->num_readers == 0) pthread_mutex_unlock(&(self->write_lock));
        pthread_mutex_unlock(&(self->fields_lock));
        return mapNode->val;
    }else{
        //READER LEAVES
        pthread_mutex_lock(&(self->fields_lock));
        self->num_readers--;
        //last reader unlocks writeLock
        if(self->num_readers == 0) pthread_mutex_unlock(&(self->write_lock));
        pthread_mutex_unlock(&(self->fields_lock));
        return MAP_VAL(NULL, 0);
    }
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    pthread_mutex_lock(&(self->write_lock));
    bool invalid = (key.key_len == 0) || (key.key_base==NULL) || (self == NULL) || (self->invalid);

    if(invalid){
        errno = EINVAL;
        pthread_mutex_unlock(&(self->write_lock));
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    int index = get_index(self, key);
    map_node_t* mapNode = &(self->nodes[index]);

    int tempIndex = index;
    bool foundKey = false;
    for(int i=0; i<self->capacity; i++){
        tempIndex %= self->capacity;
        mapNode = &(self->nodes[tempIndex]);

        //if we run into tombstone or NULL, does not exist
        bool notFound = (mapNode->key.key_base == NULL) || (mapNode->key.key_len == 0) ||
                        (mapNode->tombstone);

        if(notFound) break;

        //check if same length and same base
        foundKey = (mapNode->key.key_len == key.key_len) &&
                    !memcmp(mapNode->key.key_base, key.key_base, key.key_len);

        if(foundKey) break;
        tempIndex++;
    }

    if(foundKey){
        deleteQ(self->q, mapNode);
        mapNode->tombstone = true;
        decrementSize(self);
        pthread_mutex_unlock(&(self->write_lock));
        return *mapNode;
    }

    pthread_mutex_unlock(&(self->write_lock));
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
}

bool clear_map(hashmap_t *self) {
    pthread_mutex_lock(&(self->write_lock));
    bool invalid = (self==NULL) || (self->invalid);
    if(invalid){
        errno = EINVAL;
        pthread_mutex_unlock(&(self->write_lock));
        return false;
    }

    map_node_t* mapNode;
    for(int i=0; i<self->capacity; i++){
        mapNode = &(self->nodes[i]);

        //if we run into tombstone or NULL, does not exist
        bool notFound = (mapNode->key.key_base == NULL) || (mapNode->key.key_len == 0) ||
                        (mapNode->tombstone);

        //reset deleted node
        if(mapNode->tombstone){
            mapNode->key.key_base = NULL;
            mapNode->key.key_len = 0;
            mapNode->val.val_base = NULL;
            mapNode->val.val_len = 0;
            mapNode->tombstone = false;
        }

        if(notFound) continue;

        self->destroy_function(mapNode->key, mapNode->val);
        mapNode->key.key_base = NULL;
        mapNode->key.key_len = 0;
        mapNode->val.val_base = NULL;
        mapNode->val.val_len = 0;
    }

    clearQ(self->q);

    pthread_mutex_lock(&(self->fields_lock));
    self->size = 0;
    pthread_mutex_unlock(&(self->fields_lock));

    pthread_mutex_unlock(&(self->write_lock));
    return true;
}

bool invalidate_map(hashmap_t *self) {
    pthread_mutex_lock(&(self->write_lock));
    bool invalid = (self==NULL) || (self->invalid);
    if(invalid){
        errno = EINVAL;
        pthread_mutex_unlock(&(self->write_lock));
        return false;
    }

    map_node_t* mapNode;
    for(int i=0; i<self->capacity; i++){
        mapNode = &(self->nodes[i]);

        //if we run into tombstone or NULL, does not exist
        bool notFound = (mapNode->key.key_base == NULL) || (mapNode->key.key_len == 0) ||
                        (mapNode->tombstone);

        if(notFound) continue;

        self->destroy_function(mapNode->key, mapNode->val);
        decrementSize(self);
    }

    free(self->nodes);

    pthread_mutex_lock(&(self->fields_lock));
    self->invalid = true;
    pthread_mutex_unlock(&(self->fields_lock));

    pthread_mutex_unlock(&(self->write_lock));
    return true;
}