#ifndef HELPER
#define HELPER

#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

//Get block size from *sf_free_header
#define SIZE(b) (b->header.block_size << 4)

//Get payload size from *sf_free_header
#define PAYLOAD_SIZE(b) (SIZE(b) - (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8)

//Get payload address from *sf_free_header
#define PAYLOAD(b) ((void*)(b + SF_HEADER_SIZE/8))

//Get footer address from *sf_free_header
#define FOOTER(b) (((void*)b) + SIZE(b) - SF_FOOTER_SIZE/8)

//Return null and set sf_errno to EINVAL
#define INVALID() sf_errno = EINVAL; return NULL;

sf_free_header *getFreeBlock(size_t size);
free_list *findFreeList(size_t size);

void *reallocSplit(size_t reqSize, void *original);
void addFreeBlock(sf_free_header *block);
void *buildFreeBlock(size_t size, void *pointer);
void *splitBlock(size_t reqSize, void *block);
sf_free_header *addMemPage();
size_t fullSize(size_t size);
void removeFreeBlock(sf_free_header *block);
sf_header *allocateFreeBlock(size_t reqSize, sf_free_header *block);
free_list *whichList(sf_free_header *block);

#endif