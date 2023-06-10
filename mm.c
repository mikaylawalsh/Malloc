#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * BEFORE GETTING STARTED:
 *
 * Familiarize yourself with the functions and constants/variables
 * in the following included files.
 * This will make the project a LOT easier as you go!!
 *
 * The diagram in Section 4.1 (Specification) of the handout will help you
 * understand the constants in mm.h
 * Section 4.2 (Support Routines) of the handout has information about
 * the functions in mminline.h and memlib.h
 */
#include "./memlib.h"
#include "./mm.h"
#include "./mminline.h"

block_t *prologue;
block_t *epilogue;

// rounds up to the nearest multiple of WORD_SIZE
static inline long align(long size) {
    return (((size) + (WORD_SIZE - 1)) & ~(WORD_SIZE - 1));
}

/*
 *                             _       _ _
 *     _ __ ___  _ __ ___     (_)_ __ (_) |_
 *    | '_ ` _ \| '_ ` _ \    | | '_ \| | __|
 *    | | | | | | | | | | |   | | | | | | |_
 *    |_| |_| |_|_| |_| |_|___|_|_| |_|_|\__|
 *                       |_____|
 *
 * initializes the dynamic storage allocator (allocate initial heap space)
 * arguments: none
 * returns: 0, if successful
 *         -1, if an error occurs
 */
int mm_init(void) {
    flist_first = NULL;

    void *holder = mem_sbrk(TAGS_SIZE);  // allocated space for the prologue

    if (holder == (void *)-1) {  // error checking mem_sbrk
        return -1;
    } else {  // setting size, allocated, and pointer for prologue
        prologue = holder;
        block_set_size_and_allocated(prologue, TAGS_SIZE, 1);
    }

    void *holder2 = mem_sbrk(TAGS_SIZE);  // allocated space for the epilogue

    if (holder2 == (void *)-1) {  // error checking mem_sbrk
        return -1;
    } else {  // setting size, allocated, and pointer for epilogue
        epilogue = holder2;
        block_set_size_and_allocated(epilogue, TAGS_SIZE, 1);
    }
    return 0;
}

/* split: splits a free block into a free block and an allocated block
arguments: b: the block to allocate
           free: the free block to split
           size: the size of the allocated block
returns: a pointer to the newly allocated block
*/

block_t *split(block_t *b, block_t *free, long size) {
    long diff = block_size(free) - size;  // determine new size of free block

    block_set_size(free, diff);
    b = block_next(free);                      // set pointer for b
    block_set_size_and_allocated(b, size, 1);  // set size and allocated for b

    return b;
}

/* coalesce: merge two free blocks if they are next to each other
arguments: b: the block to coalesce with its neighbors
returns: nothing
*/
void coalesce(block_t *b) {
    if (!block_next_allocated(b)) {  // checks if the next block is free
        long size = block_size(b);
        block_t *next = block_next(b);
        pull_free_block(next);  // pulls next block from free list
        block_set_size_and_allocated(b, block_size(next) + size,
                                     0);  // resets size of current block
    }
    if (!block_prev_allocated(b) ||
        block_prev(b) ==
            b) {  // checks if prev block is free or if b is first block in heap
        long size = block_size(b);
        block_t *prev = block_prev(b);
        pull_free_block(prev);  // pulls prev from free list
        block_set_size_and_allocated(prev, block_size(prev) + size,
                                     0);  // resets size of prev
        b = prev;                         // sets pointer
        block_set_allocated(b, 0);
    }
    insert_free_block(b);
}

/*     _ __ ___  _ __ ___      _ __ ___   __ _| | | ___   ___
 *    | '_ ` _ \| '_ ` _ \    | '_ ` _ \ / _` | | |/ _ \ / __|
 *    | | | | | | | | | | |   | | | | | | (_| | | | (_) | (__
 *    |_| |_| |_|_| |_| |_|___|_| |_| |_|\__,_|_|_|\___/ \___|
 *                       |_____|
 *
 * allocates a block of memory and returns a pointer to that block's payload
 * arguments: size: the desired payload size for the block
 * returns: a pointer to the newly-allocated block's payload (whose size
 *          is a multiple of ALIGNMENT), or NULL if an error occurred
 */
void *mm_malloc(long size) {
    if (size == 0) {
        return NULL;
    }
    size = align(size) + TAGS_SIZE;  // aligns and adds header and footer
    block_t *new_block;

    if (flist_first != NULL) {
        block_t *cur = block_flink(flist_first);
        do {
            if (block_size(cur) - size >=
                MINBLOCKSIZE) {  // checks if free block is big enough to split
                new_block = NULL;
                new_block = split(new_block, cur, size);  // splits free block
                return new_block->payload;
            } else if (size <=
                       block_size(
                           cur)) {  // checks if free block is bigger than size,
                                    // but not big enough to split
                pull_free_block(cur);
                block_set_allocated(cur, 1);
                return cur->payload;
            }
            cur = block_flink(cur);  // go to next free block in free list
        } while (cur != flist_first);
    }
    int sbrk = 512;
    if (size > 512) {
        sbrk = size;
    }
    block_t *holder =
        mem_sbrk(sbrk);  // allocates enough space of size or 512 bytes

    if (holder == (void *)-1) {  // error checks mem_sbrk
        return NULL;
    }

    block_t *old_epilogue = epilogue;
    block_set_size_and_allocated(old_epilogue, sbrk,
                                 1);  // sets size and allocated of old_epilogue
                                      // which is entire size of new block
    epilogue = block_next(old_epilogue);  // sets new pointer of epilogue
    block_set_size_and_allocated(epilogue, TAGS_SIZE,
                                 1);  // sets size and allocated of epilogue

    if (sbrk - size >= MINBLOCKSIZE) {  // check if we can split
        block_set_size_and_allocated(old_epilogue, sbrk - size, 0);
        insert_free_block(old_epilogue);
        block_t *start_old = block_next(old_epilogue);  // new allocated block
        block_set_size_and_allocated(start_old, size, 1);
        return start_old->payload;
    } else {  // no splitting
        block_set_allocated(old_epilogue, 1);
        return old_epilogue->payload;
    }
}

/*                              __
 *     _ __ ___  _ __ ___      / _|_ __ ___  ___
 *    | '_ ` _ \| '_ ` _ \    | |_| '__/ _ \/ _ \
 *    | | | | | | | | | | |   |  _| | |  __/  __/
 *    |_| |_| |_|_| |_| |_|___|_| |_|  \___|\___|
 *                       |_____|
 *
 * frees a block of memory, enabling it to be reused later
 * arguments: ptr: pointer to the block's payload
 * returns: nothing
 */
void mm_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    block_t *my_block = payload_to_block(ptr);

    if (block_prev_allocated(my_block) &&
        block_next_allocated(
            my_block)) {  // checks if prev and next are allocated
        block_set_allocated(my_block, 0);
        insert_free_block(my_block);
    } else {  // else coalesce
        block_set_allocated(my_block, 0);
        coalesce(my_block);
        return;
    }
}

/*
 *                                            _ _
 *     _ __ ___  _ __ ___      _ __ ___  __ _| | | ___   ___
 *    | '_ ` _ \| '_ ` _ \    | '__/ _ \/ _` | | |/ _ \ / __|
 *    | | | | | | | | | | |   | | |  __/ (_| | | | (_) | (__
 *    |_| |_| |_|_| |_| |_|___|_|  \___|\__,_|_|_|\___/ \___|
 *                       |_____|
 *
 * reallocates a memory block to update it with a new given size
 * arguments: ptr: a pointer to the memory block's payload
 *            size: the desired new payload size
 * returns: a pointer to the new memory block's payload
 */

void *mm_realloc(void *ptr, long size) {
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    block_t *cur = payload_to_block(ptr);
    long cur_b_size = block_size(cur);
    long nsize = align(size) + TAGS_SIZE;
    if (nsize <= cur_b_size) {  // checks if we are decreasing size
        if (cur_b_size - nsize >= MINBLOCKSIZE) {  // check if we can split
            block_set_size(cur, nsize);
            block_set_size_and_allocated(block_next(cur), cur_b_size - nsize,
                                         0);
            insert_free_block(block_next(cur));
        }
        return cur->payload;

    } else {                               // nsize is bigger than current size
        if (!block_next_allocated(cur)) {  // checks if next block is free
            long tot_size = block_next_size(cur) + cur_b_size;
            long diff = tot_size - nsize;
            if (diff >= 0) {  // checks if next block has enough space
                if (diff >= MINBLOCKSIZE) {  // check if we can split
                    pull_free_block(block_next(cur));
                    block_set_size_and_allocated(cur, nsize, 1);
                    block_set_size_and_allocated(block_next(cur), diff, 0);
                    insert_free_block(block_next(cur));
                } else {  // if we can't split
                    pull_free_block(block_next(cur));
                    block_set_size(cur, tot_size);
                }
                return cur->payload;
            }
        }

        if (!block_prev_allocated(cur)) {  // checks if prev is free
            long tot_size = block_size(block_prev(cur)) + cur_b_size;
            long diff = tot_size - nsize;
            void *ret;
            if (diff >= 0) {                 // check if prev has enough space
                if (diff >= MINBLOCKSIZE) {  // check if we can split
                    block_t *prev = block_prev(cur);
                    block_set_size_and_allocated(prev, diff, 0);
                    block_set_size_and_allocated(block_next(prev), nsize, 1);
                    ret = memmove((char *)(prev->payload) + diff, ptr,
                                  cur_b_size - TAGS_SIZE);
                } else {  // if we can't split
                    pull_free_block(block_prev(cur));
                    block_set_size_and_allocated(block_prev(cur), tot_size, 1);
                    ret = memmove(block_prev(cur)->payload, ptr,
                                  cur_b_size - TAGS_SIZE);
                }
                return ret;
            }
        }
        if (!block_next_allocated(cur) &&
            !block_prev_allocated(cur)) {  // check if next and prev and free
            long tot_size =
                block_next_size(cur) + block_prev_size(cur) + cur_b_size;
            long diff = tot_size - nsize;
            void *ret;
            if (diff >= 0) {  // checks if next and prev have enough space
                if (diff >= MINBLOCKSIZE) {  // check if we can split
                    pull_free_block(block_next(cur));
                    block_t *prev = block_prev(cur);
                    block_set_size_and_allocated(prev, diff, 0);
                    block_set_size_and_allocated(block_next(prev), nsize, 1);
                    ret = memmove((char *)(prev->payload) + diff, ptr,
                                  cur_b_size - TAGS_SIZE);
                } else {  // if we can't split
                    pull_free_block(block_next(cur));
                    block_set_size_and_allocated(block_prev(cur), tot_size, 1);
                    ret = memmove(block_prev(cur)->payload, ptr,
                                  cur_b_size - TAGS_SIZE);
                }
                return ret;
            }
        }
        block_t *new_block = mm_malloc(nsize);
        if (new_block == NULL) {  // error check malloc
            return NULL;
        }
        new_block = memmove(new_block, ptr, align(size));
        mm_free(ptr);
        return new_block;
    }
}