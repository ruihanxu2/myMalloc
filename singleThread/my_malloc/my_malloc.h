#ifndef MY_MALLOC_H
#define MY_MALLOC_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
typedef struct block{
  size_t size;
  struct block* prev;
  struct block* next;
  void* data;
  int fr; // 0 : not free; 1 : free
}block_t;

static block_t* head = NULL;
static block_t* tail = NULL;
static void* start_addr = 0;
static int first = 0;

void* new_block(size_t size, block_t* last);
void parition(block_t* fitted_block, size_t size);
block_t* search_ff(block_t* head, size_t size);
void *ff_malloc(size_t size);
block_t* search_bf(block_t* head, size_t size);
void *bf_malloc(size_t size);
void add_to_ll(block_t* cur);
void merge_rstbrk();
void ff_free(void *ptr);
void bf_free(void *ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size(); 

#endif
