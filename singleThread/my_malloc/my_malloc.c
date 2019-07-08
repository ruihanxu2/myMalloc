#include "my_malloc.h"
#include <limits.h>
#include <assert.h>
//after allocation, we need to check if the remaining block is large enough                                     
//if it is (>meta), we need to create a new block which is free                                                 
//if it is not, we may simply return the block with that part of the space  
//
//typedef struct block{
//	size_t size;
//	block_t* prev;
//	block_t* next;
//	block_t* prev_f;
//	block_t* next_f;
//	void* data;
//	int fr; // 0 : not free; 1 : free
//}block_t;
//
//static block_t* head = NULL;
//void* start_addr = 0;
//static int first = 0;

void* new_block(size_t size, block_t* last){
  //new a block here using sbrk()
  if(first == 0){
    first = 1;
    start_addr = sbrk(0);
  }
  void* newblock_whole = sbrk(sizeof(block_t)+size);
	
  //error handling
  if(newblock_whole == (void *) -1){
    perror("Cannot sbrk()");
    return NULL;
  }
  block_t* newblock = newblock_whole;
	
  //newblock is a pointer to info
	
  newblock->size = size;// size is from user
  newblock->fr = 0;//not free
  newblock->data = newblock_whole+sizeof(block_t);
  //not in free list;
  newblock->next = NULL;
  newblock->prev = NULL;
  return newblock;
}


void parition(block_t* fitted_block, size_t size){
  if(fitted_block == NULL) return;
  //change the status of the fitted_block(size, fr, prev, next..)
  //need to be careful if the fitted_block is tail
  int partitioned = 0;
  if(fitted_block->size > sizeof(block_t)+size){
    partitioned = 1;
    //the left space can be a new block
    block_t* second_block = (block_t*)(fitted_block->data + size);
		
		
    second_block->fr = 1; //free to be used
    second_block->size = fitted_block->size - size - sizeof(block_t);
    second_block->data = fitted_block->data + size + sizeof(block_t);
		
    fitted_block->fr = 0; // allocated, not free
    fitted_block->size = size; // new size
    //add the second block in to the linked list
    //the partitioned block cannot be the head;

    if(fitted_block->next == NULL){
      //last one in the ll
      tail = second_block;
      assert(tail!=NULL);
      second_block->next = NULL;
      second_block->prev = fitted_block;
      fitted_block->next = second_block;
    }
		
    //fitted_block->next != NULL
    else{
      //tail not change
      //in the middle, fitted_block->next 
      fitted_block->next->prev = second_block;
      second_block->next = fitted_block->next;
      second_block->prev = fitted_block;
      fitted_block->next = second_block;
			
    }
		
  }
  //remove fitted_block from ll
  // if not partitioned, tail = fitted->prev
	
  fitted_block->fr = 0;
  if(fitted_block->next == NULL&&fitted_block->prev == NULL){
    head == NULL;
    if(partitioned == 0) tail = fitted_block->prev;
  }
  else if(fitted_block->next != NULL&&fitted_block->prev == NULL){
    //fitted_block is head;
    head  = fitted_block->next;
    fitted_block->next->prev = NULL;
    fitted_block->next = NULL;
    assert(tail!=NULL);
  }
  else if(fitted_block->next == NULL&&fitted_block->prev != NULL){
    //fitted_block is the tail of ll;
    if(partitioned == 0) tail = fitted_block->prev;
    fitted_block->prev->next = NULL;
    fitted_block->prev = NULL;
		
    assert(tail!=NULL);
  }
  else{
    //middle
    fitted_block->prev->next = fitted_block->next;
    fitted_block->next->prev = fitted_block->prev;
    fitted_block->prev = NULL;
    fitted_block->next = NULL;
    assert(tail!=NULL);
  }
}


//add a thing that recodes smallest in ll
block_t* search_ff(block_t* head, size_t size){
  if(head == NULL) return NULL;
  block_t * cur = head;
  while(cur){
    if(cur->size >= size){
      //in partition, remove cur from ll;
      //add second into ll(if exists)
      parition(cur, size);
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}


//First Fit malloc/free
void *ff_malloc(size_t size){
  if(first == 0){
    // this is the very first alloc
    block_t* newblk = new_block(size, NULL);
    if(!newblk) return NULL; //failed
    return newblk->data;
  }
  block_t* res = search_ff(head, size);
  if(res == NULL){
    block_t* newblk = new_block(size, NULL);
    if(!newblk) return NULL; //failed
    return newblk->data;
  }
  else{
    //we have searched a free block that fits
    return res->data;
  }
}


block_t* search_bf(block_t* head, size_t size){
  if(head == NULL) return NULL;
  block_t * res = NULL;
  block_t * cur = head;
  int diff = INT_MAX;
  while(cur){
    //best fit
    if(cur->size == size){
      parition(cur, size);
      return cur;
    }
    else if(cur->size > size){
      if((cur->size - size) < diff){
	res = cur;
	diff = cur->size - size;
      }
    }
    cur = cur->next;
  }
  parition(res, size);
  return res;
}


void *bf_malloc(size_t size){
  if(first == 0){
    // this is the very first alloc
    block_t* newblk = new_block(size, NULL);
    if(!newblk) return NULL; //failed
    return newblk->data;
  }
  block_t* res = search_bf(head, size);
  if(res == NULL){
    block_t* newblk = new_block(size, NULL);
    if(!newblk) return NULL; //failed
    return newblk->data;
  }
  else{
    //we have searched a free block that fits
    //remove it from ll
    return res->data;
  }
}

void add_to_ll(block_t* cur){
  if(head == NULL){
    //cur need to be the new head
    cur->prev = NULL;
    cur->next = NULL;
    head = cur;	
    tail = cur;
  }
  else if(head>cur){
    //cur is before head
    cur->prev = NULL;
    cur->next = head;
    head->prev = cur;
    head = cur;
  }
  else{
    //we need to find the free block before and after cur..
    //what if we add the free block point to those that are not free
    block_t* dummy = head;
		
    while(dummy->next && dummy->next<cur){
      dummy = dummy ->next;
    }
		
    if(dummy->next == NULL){
      //append at the end;
      dummy->next = cur;
      cur->prev = dummy;
      cur->next = NULL;
      tail = cur;
    }
    else{
      //dummy->next will be the one after
      //dummy is the one before
      cur->next = dummy->next;
      cur->prev = dummy;
      dummy->next->prev = cur;
      dummy->next = cur;
			
    }
  }
}
//Best Fit malloc/free
//this function merges the free blocks, which should only happen at block_to_free
void merge_rstbrk(block_t* cur){
  if(cur->next && cur->data + cur->size  == cur->next){
    //consec
		
    if(cur->next->next){
      cur->next->next->prev = cur;
      cur->size = sizeof(block_t) + cur->next->size+ cur->size;
      cur->next = cur->next->next;
    }
    else{
      //cur->next == tail
			
      cur->size = sizeof(block_t) + cur->next->size+ cur->size;
      cur->next = NULL;
      tail = cur;
    }
				
				
  }
  if(cur->prev && cur-> prev->data+ cur->prev->size  == cur){
    block_t* pre = cur->prev;
    if(pre->next->next){
      pre->next->next->prev = pre;
      pre->size = sizeof(block_t) + pre->next->size+ pre->size;
      pre->next = pre->next->next;
    }
    else{
      pre->size = sizeof(block_t) + pre->next->size+ pre->size;
      pre->next = NULL;
      tail = pre;
      assert(tail!=NULL);
    }
			
			
  }

  void *endAddress = sbrk(0);
	
  //find ll tail
  block_t* dummy = tail;

  if(dummy->data + dummy->size == endAddress){
    // remove dummy from ll
    tail = dummy->prev;
    if(dummy == head){
      head = NULL;
			
    }
    else{
      dummy->prev->next = NULL;
    }
    brk(dummy);
  }
}

void ff_free(void *ptr){
  if(ptr == NULL) return;
  block_t* block_to_free = (block_t* )(ptr - sizeof(block_t));
  block_to_free->fr = 1;//useless...
  add_to_ll(block_to_free);
  merge_rstbrk(block_to_free);

}
void bf_free(void *ptr){
  ff_free(ptr);
}

unsigned long get_data_segment_size(){
  void *endAddress = sbrk(0);
  unsigned long res = (unsigned long)endAddress - (unsigned long)start_addr;
  return res;
} //in bytes

unsigned long get_data_segment_free_space_size(){
  unsigned long res = 0;
  block_t* dummy = head;
  while(dummy){
    res += dummy->size+sizeof(block_t);
    dummy = dummy->next;
  }
  return res;
} //in bytes

