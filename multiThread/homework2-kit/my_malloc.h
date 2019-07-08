#ifndef MY_MALLOC_H
#define MY_MALLOC_H
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>

typedef struct block{
  size_t size;
  struct block* next;
  void* data;
}block_t;

__thread block_t* head_nolock = NULL;
__thread block_t* tail_nolock = NULL;


static block_t* head = NULL; //header
static block_t* tail = NULL;

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t lock;
pthread_mutex_t freelist_lock;
pthread_mutex_t sbrk_lock;

//Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

#endif
