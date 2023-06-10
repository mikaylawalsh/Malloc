#ifndef MEMLIB_H
#define MEMLIB_H

#include <unistd.h>

void mem_init(void);
void mem_deinit(void);
void *mem_sbrk(int incr);
void mem_reset_brk(void);
void *mem_heap_lo(void);
void *mem_heap_hi(void);
long mem_heapsize(void);
long mem_pagesize(void);

#endif
