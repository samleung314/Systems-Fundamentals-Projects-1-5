/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "helper.h"
#include "debug.h"

int pageCount = 0;

/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
	{NULL, LIST_1_MIN, LIST_1_MAX},
	{NULL, LIST_2_MIN, LIST_2_MAX},
	{NULL, LIST_3_MIN, LIST_3_MAX},
	{NULL, LIST_4_MIN, LIST_4_MAX}
};

int sf_errno = 0;

void *sf_malloc(size_t size) {
	//Check valid request size
	if(size == 0 || size > 4*PAGE_SZ) {
		sf_errno = EINVAL;
		return NULL;
	}

	//Find a fitting block
	size_t foundBlockSize = 0;
	sf_free_header *freeBlock;

	while(foundBlockSize < size){
		freeBlock = getFreeBlock(size);

		if(freeBlock == NULL){
			sf_errno = ENOMEM;
			return NULL;
		}

		foundBlockSize = PAYLOAD_SIZE(freeBlock);
	}

	//Decide to split
	int split = ((SIZE(freeBlock) - fullSize(size)) > 32);
	if(split){
		freeBlock = splitBlock(size, freeBlock);
	}else{
		freeBlock = (sf_free_header*)allocateFreeBlock(size, freeBlock);
	}

	// pointer to payload
	return ((void*)freeBlock + SF_HEADER_SIZE/8);
}

void *sf_realloc(void *ptr, size_t size) {
	//Check invalid pointer
	if(ptr == NULL) abort();

	//Check within heap
	sf_header *head = ptr - SF_HEADER_SIZE/8;
	void *blockEnd = (void*)head + (head->block_size << 4);
	sf_footer *foot = blockEnd - (SF_FOOTER_SIZE/8);
	if(ptr < get_heap_start() || blockEnd > get_heap_end()){{INVALID();}}

	//Check if alloc bits are 0
	if(!(head->allocated) || !(foot->allocated)) {INVALID();}

	//Check requested_size, block_size, padded make sense together
	size_t requested_size =	foot->requested_size;
	size_t block_size =		head->block_size << 4;
	int padded = 			head->padded;
	size_t payload = 		block_size - (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8;

	if(padded){
		//Payload must be bigger than requested if padded
		if(requested_size >= payload) {INVALID();}
	}else{
		//Payload must be equal requested size
		if(requested_size != payload) {INVALID();}
	}

	//Check padded and alloc in header and footer
	if(head->padded != foot->padded) {INVALID();}
	if(head->allocated != foot->allocated) {INVALID();}

	//Free if realloc to size 0
	if(size == 0){
		sf_free(ptr);
		return NULL;
	}

	//Decide reallocating to larger or smaller or same
	size_t reallocBlockSize = fullSize(size);
	//size_t reallocPayload = PAYLOAD_SIZE(reallocBlockSize);

	//Reallocate to larger block
	if(reallocBlockSize > block_size){
		//Obtain larger block, copy data, free ptr
		void *memoryRegion = sf_malloc(size);

		if(memoryRegion == NULL) return NULL;

		memcpy(memoryRegion, ptr, payload);
		sf_free(ptr);
		return memoryRegion;

	//Reallocate to smaller block
	}else if(reallocBlockSize < block_size){
		//Decide to split
		int split = ((block_size - fullSize(size)) > 32);
		if(split){
			return reallocSplit(size, ptr);

		//No split, just set padded and requested_size bits
		}else{
			//Set header
			int isPadded = ((reallocBlockSize - (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8) > size);
			head->padded = isPadded;

			//Set footer
			foot->padded = isPadded;
			foot->requested_size = size;
			return ptr;
		}


	//No reallocation if block sizes will be same anyways
	}else if(reallocBlockSize == block_size){
		//No change of requested size if they are the same
		if(requested_size == size){
			return ptr;

		//Change requested size to match realloc size in footer
		}else{
			int isPadded = ((reallocBlockSize - (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8) > size);
			head->padded = isPadded;

			foot->padded = isPadded;
			foot->requested_size = size;
			return ptr;
		}

	}

	return NULL;
}

void sf_free(void *ptr) {
	//Check invalid pointer
	if(ptr == NULL) abort();

	//Check within heap
	sf_header *head = ptr - SF_HEADER_SIZE/8;
	void *blockEnd = (void*)head + (head->block_size << 4);
	sf_footer *foot = blockEnd - (SF_FOOTER_SIZE/8);
	if(ptr < get_heap_start() || blockEnd > get_heap_end()) abort();

	//Check if alloc bits are 0
	if(!(head->allocated) || !(foot->allocated)) abort();

	//Check requested_size, block_size, padded make sense together
	size_t requested_size =	foot->requested_size;
	size_t block_size =		head->block_size << 4;
	int padded = 			head->padded;
	size_t payload = 		block_size - (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8;

	if(padded){
		//Payload must be bigger than requested if padded
		if(requested_size >= payload) abort();
	}else{
		//Payload must be equal requested size
		if(requested_size != payload) abort();
	}

	//Check padded and alloc in header and footer
	if(head->padded != foot->padded) abort();
	if(head->allocated != foot->allocated) abort();

	//Check if can coalesce next
	sf_free_header *nextHead = blockEnd;

	bool endOfHeap = ((void*)nextHead >= get_heap_end());

	if(!nextHead->header.allocated && !endOfHeap){
		//Remove the next block from lists
		removeFreeBlock(nextHead);

		//Build new block, and add to lists
		size_t newPayload = ((head->block_size << 4)  + (nextHead->header.block_size << 4) - (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8);
		addFreeBlock(buildFreeBlock(newPayload, head));

	//If cannot coalesce, just free
	}else{
		size_t freePayload = ((head->block_size << 4) - (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8);
		addFreeBlock(buildFreeBlock(freePayload, head));
	}
}

//Split function for realloc
void *reallocSplit(size_t reqSize, void *original){
	void *headAddress = original - SF_HEADER_SIZE/8;
	sf_header *originalHeader = headAddress;
	sf_footer *originalFoot = headAddress + (originalHeader->block_size << 4) - SF_FOOTER_SIZE/8;
	int toSplit = fullSize(reqSize);

	size_t remainderBlockSize = (originalHeader->block_size << 4) - toSplit;

	//Build split-off block
	int isPadded = ((toSplit - (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8) > reqSize);

	originalHeader->padded = isPadded;
	originalHeader->block_size = toSplit >> 4;

	sf_footer *splitFoot = headAddress + toSplit - SF_FOOTER_SIZE/8;
	sf_footer newFoot;
	newFoot.allocated = 1;
	newFoot.padded = isPadded;
	newFoot.two_zeroes = 0;
	newFoot.block_size = toSplit >> 4;
	newFoot.requested_size = reqSize;
	*splitFoot = newFoot;

	int fakeRequested = remainderBlockSize - (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8;
	//Build fake allocated remainder block to be freed
	sf_header *newHead = headAddress + toSplit;
	newHead->allocated = 1;
	newHead->padded = 0;
	newHead->two_zeroes = 0;
	newHead->block_size = remainderBlockSize >> 4;
	newHead->unused = 0;

	originalFoot->allocated = 1;
	originalFoot->padded = 0;
	originalFoot->two_zeroes = 0;
	originalFoot->block_size = toSplit >> 4;
	originalFoot->requested_size = fakeRequested;

	sf_free(headAddress + toSplit + SF_HEADER_SIZE/8);

	return original;
}

//Returns header of found free block
sf_free_header *getFreeBlock(size_t size){
	//Add sizes of header and footer and necessary padding
	size = fullSize(size);

	//Search chosen free list or search other lists
	for(int i=0; i<FREE_LIST_COUNT; i++){
		sf_free_header *pointer = seg_free_list[i].head;

		//Advance pointer through list until we find free block
		while(pointer != NULL){
			if((pointer->header.block_size << 4) >= size){
				return pointer;
			}
			pointer = pointer->next; //Advance pointer to next linked block
		}
	}

	//No free block found, add a mem page only if there's less than 4
	sf_free_header *memPage = addMemPage();
	if(memPage == NULL){
		sf_errno = ENOMEM;
		return NULL;
	}

	return memPage;
}

//Builds a free block of specified size at specified address
//Returns pointer at free header of built block
void *buildFreeBlock(size_t size, void *pointer){

	//Add sizes of header and footer and necessary padding
	int padding = 0;
	if(size % 16) padding = 16 - (size % 16);
	int payload = size + padding;

	size += (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8 + padding;

	sf_free_header newHead;
	newHead.header.allocated = 0;
	newHead.header.padded = (padding > 0);
	newHead.header.two_zeroes = 0;
	newHead.header.block_size = size >> 4;
	newHead.header.unused = 0;

	newHead.next = NULL;
	newHead.prev = NULL;

	sf_footer newFoot;
	newFoot.allocated = 0;
	newFoot.padded = (padding > 0);
	newFoot.two_zeroes = 0;
	newFoot.block_size = size >> 4;
	newFoot.requested_size = 0;

	*((sf_free_header*)pointer) = newHead;
	*((sf_footer*)(pointer + SF_HEADER_SIZE/8 + payload)) = newFoot;

	return pointer;
}

sf_header *allocateFreeBlock(size_t size, sf_free_header *block){
	//Remove block from its list
	removeFreeBlock(block);

	//Set header
	int isPadded = (PAYLOAD_SIZE(block) > size);

	block->header.allocated = 1;
	block->header.padded = isPadded;

	block->next = NULL;
	block->prev = NULL;

	//Set footer
	sf_footer *foot = (sf_footer*)FOOTER(block);
	foot->allocated = 1;
	foot->padded = isPadded;
	foot->requested_size = size;

	return (sf_header*)block;
}

//Adds free block to appropriate list
void addFreeBlock(sf_free_header *block){
	//Find fitting free list for the size
	int size = block->header.block_size << 4;
	free_list *searchList;

	for(int i=0; i<FREE_LIST_COUNT; i++){
		if(size >= seg_free_list[i].min && size <= seg_free_list[i].max){
			searchList = &seg_free_list[i];
			break;
		}
	}

	//Add free block to free list
	if(searchList->head == NULL){
		searchList->head = block;		//Set head to block if list is empty
	}else{
		searchList->head->prev = block;	//Set head prev to new block
		block->next = searchList->head;	//Set new block next to old head
		searchList->head = block;		//Set list header to new block
	}
}

//Remove free block from list
void removeFreeBlock(sf_free_header *block){
	//Case 1: Header of list
	if(block->prev == NULL){
		free_list *list = whichList(block);

		//If only element of list
		if(block->next == NULL){
			list->head = NULL;

		//If node exists after head, make it the head
		}else{
			block->next->prev = NULL;
			list->head = block->next;
		}

	//Case 2: End of list
	}else if(block->next == NULL){
		block->prev->next = NULL;
	//Case 3: Within list
	}else{
		block->prev->next = block->next;
		block->next->prev = block->prev;
	}
}

//Builds a free block from new memory page
//Returns header of memory page free block
sf_free_header *addMemPage(){
	//Check if more than 4 pages
	void *pointer = sf_sbrk();
	if((long int)pointer == -1 || (pageCount == 4)){
		return NULL;
	}
	pageCount++;

	//Coalesce backwards if last block not allocated
	size_t newSize = PAGE_SZ;
	sf_footer *prevBlock = pointer - SF_FOOTER_SIZE/8; //Address of prev block footer
	sf_footer *newFooter = pointer + (PAGE_SZ - SF_FOOTER_SIZE/8); //Address of new page footer

	//Decide to coalesce
	if(((void*)prevBlock > get_heap_start()) && !(prevBlock->allocated)){
		size_t prevBlockSize = prevBlock->block_size << 4;
		sf_free_header *prevHead = pointer - prevBlockSize;
		removeFreeBlock(prevHead);

		newSize += prevBlockSize;	//Add prev block's size
		pointer -= prevBlockSize; 	//Move pointer to head of prev block
	}

	//Build block of mem page

	//Build header
	sf_free_header newHead;
	newHead.header.allocated = 0;
	newHead.header.padded = 0;
	newHead.header.two_zeroes = 0;
	newHead.header.block_size = newSize >> 4;
	newHead.header.unused = 0;

	//Set link pointers to null
	newHead.next = NULL;
	newHead.prev = NULL;

	//Build footer
	sf_footer newFoot;
	newFoot.allocated = 0;
	newFoot.padded = 0;
	newFoot.two_zeroes = 0;
	newFoot.block_size = newSize >> 4;
	newFoot.requested_size = 0;

	//Set addresses to header and footer
	*(sf_free_header*)pointer = newHead;
	*newFooter = newFoot;

	//Add block to free list
	addFreeBlock(pointer);

	return pointer;
}

//Returns header of split off block
void *splitBlock(size_t reqSize, void *original){
	sf_free_header *originalHeader = ((sf_free_header*)original);
	int toSplit = fullSize(reqSize);

	size_t splittedSize = SIZE(originalHeader) - toSplit; //New size of original splitted block

	//Remove block-to-split from its list
	removeFreeBlock(original);

	//Build split-off block
	sf_free_header *splitOff = buildFreeBlock(reqSize, originalHeader);

	//Allocate split-off block
	allocateFreeBlock(reqSize, splitOff);

	//Rebuild remainder into free block
	splittedSize -= (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8;	//Subtract header and footer
	sf_free_header *split = buildFreeBlock(splittedSize, (original + toSplit));

	//Add remainder back to a list
	addFreeBlock(split);

	return splitOff;
}

//Return full size of block from requested size
size_t fullSize(size_t size){
	int padding = 0;
	if(size % 16) padding = 16 - (size % 16);

	size += (SF_HEADER_SIZE + SF_FOOTER_SIZE)/8 + padding;

	return size;
}

//Return free_list of given sf_free_header
free_list *whichList(sf_free_header *block){
	size_t size = SIZE(block);

	for(int i=0; i<FREE_LIST_COUNT; i++){
		if(size >= seg_free_list[i].min && size <= seg_free_list[i].max){
			return &seg_free_list[i];
		}
	}

	return NULL;
}