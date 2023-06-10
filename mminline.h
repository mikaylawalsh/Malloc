#ifndef MMINLINE_H_
#define MMINLINE_H_
#include <assert.h>
#include "mm.h"

static block_t *flist_first;  // head of circular, doubly linked free list
extern block_t *prologue;
extern block_t *epilogue;

/**
 * In order to pass the tests sequentially, you must implement the inline
 * functions in the order they appear here. Comments between the functions
 * list what tests you should be passing once you've successfully implemented
 * tests up to that point.
 */

/*
 *
 * |_ | _  _ ;_/     _.|| _  _  _.-+- _  _|
 * [_)|(_)(_ | \____(_]||(_)(_ (_] | (/,(_]
 *
 * returns 1 if block is allocated, 0 otherwise
 * In other words, returns 1 if the right-most bit in block's size is set, 0
 * otherwise
 */
static inline int block_allocated(block_t *b) {
 
   return b->size & 1; 
}

/*
 * |_ | _  _ ;_/     _ ._  _|    -+- _  _
 * [_)|(_)(_ | \____(/,[ )(_]____ | (_](_]
 *                                     ._|
 * returns a pointer to the block's end tag (You probably won't need to use this
 * directly)
 */
static inline long *block_end_tag(block_t *b) {
   assert(b->size >= (WORD_SIZE * 2));
 
   return (long *)(&b->payload[b->size/8 - 2]); 
}

/*
 * |_ | _  _.;_/     _ ._  _|     _.|| _  _. _.-+- _  _|
 * [_)|(_)(_.| \____(/,[ )(_]____(_]||(_)(_.(_] | (/,(_]
 *
 * same as the above, but checks the end tag of the block
 * NOTE: since b->size is divided by WORD_SIZE, the 3 right-most bits are
 * truncated (including the 'is-allocated' bit)
 */
static inline int block_end_allocated(block_t *b) {

   long end_size = b->payload[b->size/8 - 2];
   return end_size & 1;
}

/*
 * |_ | _  _.;_/     __ _ -+-     __*__. _
 * [_)|(_)(_.| \_____) (/, | _____) | /_(/,
 *
 * Sets the entire size of the block at both the beginning and the end tags
 * Preserves the alloc bit (if b is marked allocated or free, it will remain
 * so).
 * NOTE: size must be a multiple of ALIGNMENT, which means that in binary, its
 * right-most 3 bits must be 0. Thus, we can check if size is a multiple of
 * ALIGNMENT by &-ing it with ALIGNMENT - 1, which is 00..00111 in binary if
 * ALIGNMENT is 8. This is where the assert statement comes from!
 * NOTE: you may want to think about using the |= operator
 */
static inline void block_set_size(block_t *b, long size) {
   assert((size & (ALIGNMENT - 1)) == 0);

   size |= block_allocated(b);
   b->size = size; 
   *block_end_tag(b) = size; 
}

/*
 * |_ | _  _.;_/    __*__. _
 * [_)|(_)(_.| \____) | /_(/,
 *
 * returns the size of the entire block
 * NOTE: Think about how to use & to remove the is-allocated bit
 * from the size
 */
static inline long block_size(block_t *b) {

   return b->size & -2;
}

/**
 * If all functions previous to this line are complete, block_size test will
 * pass
 */

/*
 * |_ | _  _.;_/     __ _ -+-     _.|| _  _. _.-+- _  _|
 * [_)|(_)(_.| \_____) (/, | ____(_]||(_)(_.(_] | (/,(_]
 *
 * Sets the allocated flags of the block, at both the beginning and the end
 * tags. NOTE: -2 is 111...1110 in binary
 */
static inline void block_set_allocated(block_t *b, int allocated) {
   assert((allocated == 0) || (allocated == 1));

   if (allocated) { //checks is allocated is 1
        b->size |= allocated;
        *block_end_tag(b) |= allocated;
   } else { //checks if allocated is 0
        b->size &= -2;
        *block_end_tag(b) &= -2;
   }
}

/**
 * If all functions previous to this line are complete, block_allocated test
 * will pass
 */

/*
 * |_ | _  _.;_/   __ _ -+-   __*__. _    _.._  _|   _.|| _  _. _.-+- _  _|
 * [_)|(_)(_.| \___) (/, | ___) | /_(/,__(_][ )(_]__(_]||(_)(_.(_] | (/,(_]
 *
 * Sets the entire size of the block and sets the allocated flags of the block,
 * at both the beginning and the end
 */
static inline void block_set_size_and_allocated(block_t *b, long size,
                                               int allocated) {

   block_set_size(b, size);
   block_set_allocated(b, allocated);
}

/**
 * If all functions previous to this line are complete, block_size_and_allocated
 * test will pass
 */

/*
 * |_ | _  _.;_/     _ ._  _|     __*__. _
 * [_)|(_)(_.| \____(/,[ )(_]_____) | /_(/,
 *
 * same as the above, but uses the end tag of the block
 */
static inline long block_end_size(block_t *b) {

   return *block_end_tag(b) & -2;
}

/**
 * If all functions previous to this line are complete, end_tag
 * test will pass
 */

/*
 * |_ | _  _.;_/    _ ._. _ .  ,   __*__. _
 * [_)|(_)(_.| \___[_)[  (/, \/ ___) | /_(/,
 *                  |
 *
 * returns the size of the previous block. NOTE: -2 is 111...1110 in binary
 */
static inline long block_prev_size(block_t *b) {

    return b->payload[-2] & -2;
}

/*
 * |_ | _  _.;_/    ._ ._. _ .  ,
 * [_)|(_)(_.| \____[_)[  (/, \/
 *                  |
 *
 * returns a pointer to the previous block
 */
static inline block_t *block_prev(block_t *b) {
   
   return (block_t *) ((char *)b - block_prev_size(b));
}

/**
 * If all functions previous to this line are complete, block_prev
 * test will pass
 */

/*
 * |_ | _  _.;_/    ._  _ \./-+-
 * [_)|(_)(_.| \____[ )(/,/'\ |
 *
 * returns a pointer to the next block
 */
static inline block_t *block_next(block_t *b) {

   return (block_t *) ((char *)b + block_size(b));
}

/**
 * If all functions previous to this line are complete, block_next
 * test will pass
 */

/*
 * ._  _.  .| _  _. _|    -+- _     |_ | _  _.;_/
 * [_)(_]\_||(_)(_](_]____ | (_)____[_)|(_)(_.| \
 * |     ._|
 *
 * given a pointer to the payload, returns a pointer to the block
 * NOTE: think about how casting might be useful
 */
static inline block_t *payload_to_block(void *payload) {

   return (block_t *) ((long *) payload - 1);
}

/**
 * If all functions previous to this line are complete, payload_to_block
 * test will pass
 */

/*
 * |_ | _  _.;_/    ._  _ \./-+-     _.|| _  _. _.-+- _  _|
 * [_)|(_)(_.| \____[ )(/,/'\ | ____(_]||(_)(_.(_] | (/,(_]
 *
 * returns 1 if the next block is allocated; 0 if not
 */
static inline int block_next_allocated(block_t *b) {

   block_t *next = block_next(b);
   return block_allocated(next);
}

/*
 * |_ | _  _.;_/    ._  _ \./-+-     __*__. _
 * [_)|(_)(_.| \____[ )(/,/'\ | _____) | /_(/,
 *
 * returns the size of the next block
 */
static inline long block_next_size(block_t *b) {

   block_t *next = block_next(b);
   return block_size(next);
}

/**
 * If all functions previous to this line are complete, next_size_and_allocated
 * test will pass
 */

/*
 * |_ | _  _.;_/    ._ ._. _ .  ,     _.|| _  _. _.-+- _  _|
 * [_)|(_)(_.| \____[_)[  (/, \/ ____(_]||(_)(_.(_] | (/,(_]
 *                  |
 *
 * returns 1 if the previous block is allocated, 0 otherwise
 */
static inline int block_prev_allocated(block_t *b) {

   return b->payload[-2] & 1;
}

/**
 * If all functions previous to this line are complete, prev_size_and_allocated
 * test will pass
 */

/*
 * |_ | _  _.;_/    |_ |*._ ;_/
 * [_)|(_)(_.| \____[_)||[ )| \
 *
 * given the input block 'b', returns b's blink, which points to the
 * previous block in the free list. NOTE: if 'b' is free, b->payload[1]
 * contains b's blink
 */
static inline block_t *block_blink(block_t *b) {
   assert(!block_allocated(b));

    return (block_t *)b->payload[1]; 
}

/*
 * |_ | _  _.;_/     __ _ -+-    |_ |*._ ;_/
 * [_)|(_)(_.| \_____) (/, | ____[_)||[ )| \
 *
 * given the inputs 'b' and 'new_blink', sets b's blink to now point
 * to new_blink, which should be the previous block in the free list
 */
static inline void block_set_blink(block_t *b, block_t *new_blink) {
   assert(!block_allocated(b) && !block_allocated(new_blink));

   b->payload[1] = (long) new_blink;
}

/**
 * If all functions previous to this line are complete, set_blink
 * test will pass
 */

/*
 * |_ | _  _.;_/    |,|*._ ;_/
 * [_)|(_)(_.| \____| ||[ )| \
 *
 * given the input block 'b', returns b's flink, which points to the next block
 * in the free list.NOTE: if 'b' is free, b->payload[0] contains b's flink
 */
static inline block_t *block_flink(block_t *b) {
   assert(!block_allocated(b));

   return (block_t *)b->payload[0]; 
}

/*
 * |_ | _  _.;_/    __ _ -+-    |,|*._ ;_/
 * [_)|(_)(_.| \____) (/, | ____| ||[ )| \
 *
 * given the inputs 'b' and 'new_flink', sets b's flink to now point
 * to new_flink, which should be the next block in the free list
 */

static inline void block_set_flink(block_t *b, block_t *new_flink) {
   assert(!block_allocated(b) && !block_allocated(new_flink));

   b->payload[0] = (long) new_flink;
}

/**
 * If all functions previous to this line are complete, set_flink
 * test will pass
 */

/*
 *
 * . _  __ _ ._.-+-    |,._. _  _     |_ | _  _.;_/
 * |[ )_) (/,[   | ____| [  (/,(/,____[_)|(_)(_.| \
 *
 * insert block into the (circularly doubly linked) free list
 * If the list is not empty, block should be inserted between
 * flist_first and the last block in the list. flist_first
 * should always be set equal to the new block.
 */
static inline void insert_free_block(block_t *fb) {
   assert(!block_allocated(fb));

   if (flist_first != NULL) { //checks if free list is empty 
      block_t *last = block_blink(flist_first);
      block_set_flink(fb, flist_first);
      block_set_blink(fb, last);
      block_set_flink(last, fb);
      block_set_blink(flist_first, fb);
   } else { //check if free list is not empty 
      block_set_flink(fb, fb);
      block_set_blink(fb, fb);
   }
   flist_first = fb;
}

/**
 * If all functions previous to this line are complete, insert_free_block
 * test will pass
 */

/*
 * ._ . .||    |,._. _  _     |_ | _  _.;_/
 * [_)(_|||____| [  (/,(/,____[_)|(_)(_.| \
 * |
 *
 * pull a block from the (circularly doubly linked) free list
 */
static inline void pull_free_block(block_t *fb) {
   assert(!block_allocated(fb));

   if (flist_first == NULL) { //checks if free list is empty 
       return;
   }
   int first = 0;

   if (flist_first == fb) { //check if fb is first element in free list 
       if (block_flink(fb) == fb && block_blink(fb) == fb) { //checks if fb is the only block in free list
           flist_first = NULL;
           return;
       }
       first = 1; //setting a flag if fb is first 
   }
   
   block_t *prev = block_blink(fb);
   block_t *next = block_flink(fb);
   block_set_blink(next, prev);
   block_set_flink(prev, next);

   if (first == 1) { //checks if fb is first 
       flist_first = next;
   }
}

/**
 * If all functions previous to this line are complete, ALL TESTS will pass
 */

#endif  // MMINLINE_H_