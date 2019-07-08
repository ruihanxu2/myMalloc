#include "my_malloc.h"

void* new_block(size_t size){
  //new a block here using sbrk()
  void* newblock_whole = sbrk(sizeof(block_t)+size);
  
  //error handling
  if(newblock_whole == (void *) -1){
    perror("Cannot sbrk()");
    return NULL;
  }
  block_t* newblock = newblock_whole;
  
  //newblock is a pointer to info
  
  newblock->size = size;// size is from user
  newblock->data = newblock_whole+sizeof(block_t);
  //not in free list;
  newblock->next = NULL;

  return newblock;
}
block_t* search_bf(size_t size){
  if(head == NULL){
    //set up a header here

    head = sbrk(sizeof(block_t));
    //error handling
    if(head == (void *) -1){
      perror("Cannot sbrk()");
      return NULL;
    }
    head->next = NULL;
    head->data = head + sizeof(block_t);
    head->size = 0;

    return NULL;
  } 

  block_t * res = NULL;
  block_t * cur = head->next;
  int diff = INT_MAX;
  while(cur){
    //best fit
    if(cur->size == size){
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
  return res;
}


void *ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  block_t* res = search_bf(size);
  if(res == NULL){
    //brk here, can be the beginning or the last

    block_t* newblk = new_block(size);
    if(!newblk) return NULL; //failed
    pthread_mutex_unlock(&lock);
    return newblk->data;
  }
  else{
    // no partitioning...
    // move res out from ll

//		if(res->size > size+sizeof(block_t)){
//			//partion here
//			res->size = res->size - size - sizeof(block_t);
//			res = (block_t*)(res + sizeof(block_t) + res->size);
//			res->size = size;
//			res->data = res+sizeof(block_t);
//
//		}
//		else{
      //direct remove
      // get the block before res
      block_t* prev = head;
      block_t* cur = head->next;
      while(cur!=res){
        prev = cur;
        cur = cur -> next;
      }
      //now cur == res
      prev->next = res->next;
      res->next = NULL;

    
    pthread_mutex_unlock(&lock);
    return res->data;
  }
}
  
void add_to_ll(block_t* cur){
  //can only free after we have a head
    block_t* pre = head;
    block_t * dummy = head->next;
    while (dummy != NULL && dummy < cur) {
        pre = dummy;
        dummy = dummy->next;
    }
    //dummy == NULL or dummy > cur 
    //无论如何都是加在pre后面。。
    //what if pre is head and cur is right next to head?
    block_t * curr = NULL;
//      if (pre!=head && pre + pre->size + sizeof(block_t) == cur) {
//        //combine with the previous node
//        pre->size += cur->size + sizeof(block_t);
//        curr = pre;
//      } else {
//        cur->next = pre->next;
//        pre->next = cur;
//        curr = cur;
//      }
//      if (curr->data + curr->size == curr->next) {
//        //combine with the next node
//        curr->size += curr->next->size + sizeof(block_t);
//        curr->next = curr->next->next;
//      }
      cur->next = pre->next;
      pre->next = cur;
      
  }
  
  
  
  
  void ts_free_lock(void *ptr){
    if(ptr == NULL) return;
    pthread_mutex_lock(&lock);
    block_t* block_to_free = (block_t* )(ptr - sizeof(block_t));
    add_to_ll(block_to_free);
    pthread_mutex_unlock(&lock);

  }
  
  /*************************************************************
  
  NOLOCK
  
  *************************************************************/
  
  
  void* new_block_nolock(size_t size){
    //new a block here using sbrk()
    
    pthread_mutex_lock(&sbrk_lock);
    void* newblock_whole = sbrk(sizeof(block_t)+size);
    
    
    //error handling
    if(newblock_whole == (void *) -1){
      perror("Cannot sbrk()");
      return NULL;
    }
    pthread_mutex_unlock(&sbrk_lock);
    block_t* newblock = newblock_whole;
    
    //newblock is a pointer to info
    
    newblock->size = size;// size is from user
    newblock->data = newblock_whole+sizeof(block_t);
    //not in free list;
    newblock->next = NULL;

    return newblock;
  }
  block_t* search_bf_nolock(size_t size){
    if(head_nolock == NULL){
      //set up a header here
      pthread_mutex_lock(&sbrk_lock);
      head_nolock = sbrk(sizeof(block_t));
      //error handling
      if(head_nolock == (void *) -1){
        perror("Cannot sbrk()");
        exit(1);
      }
      pthread_mutex_unlock(&sbrk_lock);
      head_nolock->next = NULL;
      head_nolock->data = head + sizeof(block_t);
      head_nolock->size = 0;


      return NULL;
    } 

    block_t * res = NULL;
    block_t * cur = head_nolock->next;
    int diff = INT_MAX;
    while(cur){
      //best fit
      if(cur->size == size){
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

    return res;
  }


  void *ts_malloc_nolock(size_t size) {
    block_t* res = search_bf_nolock(size);
    if(res == NULL){
      //brk here, can be the beginning or the last

      block_t* newblk = new_block_nolock(size);
      if(!newblk) return NULL; //failed
      return newblk->data;
    }
    else{
      //find in ll, remove it
      // no partitioning...

        block_t* prev = head_nolock;
        block_t* cur = head_nolock->next;
        while(cur!=res){
          prev = cur;
          cur = cur -> next;
        }
        //now cur == res
        prev->next = res->next;
  //		}
      res->next = NULL;
      return res->data;
    }
  }
    
  void add_to_ll_nolock(block_t* cur){
      //can only add to the thread local ll
      if(head_nolock == NULL){
        //set up a header here
        pthread_mutex_lock(&sbrk_lock);
        head_nolock = sbrk(sizeof(block_t));
        //error handling
        if(head_nolock == (void *) -1){
          perror("Cannot sbrk()");
          exit(1);
        }
        pthread_mutex_unlock(&sbrk_lock);
        head_nolock->next = NULL;
        head_nolock->data = head + sizeof(block_t);
        head_nolock->size = 0;
      } 
      
      block_t* pre = head_nolock;
      block_t * dummy = head_nolock->next;
      while (dummy != NULL && dummy < cur) {
          pre = dummy;
          dummy = dummy->next;
      }
      //dummy == NULL or dummy > cur 
      //will be after pre anyways...
//      block_t * curr = NULL;
//        if (pre!=head && pre + pre->size + sizeof(block_t) == cur) {
//          //combine with the previous node
//          pre->size += cur->size + sizeof(block_t);
//          curr = pre;
//        } else {
//          cur->next = pre->next;
//          pre->next = cur;
//          curr = cur;
//        }
//        if (curr->data + curr->size == curr->next) {
//          //combine with the next node
//          curr->size += curr->next->size + sizeof(block_t);
//          curr->next = curr->next->next;
//        }
        
        cur->next = pre->next;
        pre->next = cur;
    }
    
    void ts_free_nolock(void *ptr){
      if(ptr == NULL) return;
      block_t* block_to_free = (block_t* )(ptr - sizeof(block_t));
      add_to_ll_nolock(block_to_free);

    }
  
