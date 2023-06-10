# 7-malloc

Description of strategy for maintaining compaction/avoiding fragmentation:

Our strategy was to call mem_sbrk on a larger size than was necessary. We always call mem_sbrk on at least 512 bytes. We have a statement on line 148 that checks if the size passed into mm_malloc is greater than 512. If it is, we let sbrk equal the size. Doing this allowed us to avoid fragmentation by allocating a large block and then splitting and coalescing as necessary to avoid empty space and extra free space in the middle of our allocated blocks. If we only call mem_sbrk on the size required (if less than 512), we could be left with several small free blocks in between our allocated blocks that aren't big enough to allocate. Specifically, this is how we limit external fragmentation. For internal fragmentation, we split and coalesce free blocks, but we only split if the free block that remains would be greater than MINBLOCKSIZE in another attempt to reduce external fragmentation. 

Your mm_realloc() implementation strategy, more specifically what optimizations or deliberate choices did you make to improve throughput and / or utilization

Generally, our basic mm_realloc implementation included some null checking then two main cases. The first was when our size after being aligned and adjusted for TAGSIZE (called nsize) was less than the current size of the block, meaning we want to shrink the block. In this acse, we check if we can split the block and create a free block of at least MINBLOCKSIZE. If so, we adjust the size of the current block, free block, and insert the free block into the free list. If not, we just return a pointer to the current block's payload. The second case we had to consider was if the new size was greater the current size of the block meaning we want allocate more space. In our basic implementation, we just malloced this new space and then moved the contents of the current block into the new block using memmove and then freeing the current block. Although this worked, it was very inefficient and produced low utility since we had a lot of unused space in the heap. There wasn't much we could do to improve this in the case when we were decreasing the size of the block since we were already splitting the block. In the second case, however, there were a couple things we could do to increase our space utilization. Essentially, we were going to check if the blocks around our current block were free in order to use that space instead of mallocing new space. First, we check if the next block is free and has enough space. If it does we use up as much space from the next block as we need and split the rest if it is greater than MINBLOCKSIZE. If next isn't free or doesn't have enough space, we go on to check if the previous block is free and has enough space. If so we use as much space as we need and split the rest from the top if it is greater than MINBLOCKSIZE. There is one extra step here to use memmove to copy the information from the current blocks pointer to the previous blocks pointer. If the prev doesn't have enough space, we check if both are free and if that is enough space. If so we repeat the steps above for both the next and prev in that order. If none of this works, we then return to our naive implementation and call malloc on the new size. The optimization we described above also applies here for mem_sbrk in order to limit the amount of calls made to it. 

Your strategy to achieve high throughput (see next point on Performance)

To achieve a high throughput, we minimized our calls to mem_sbrk. Initializing a block size of at least 512 allowed us to do this. As throughput is the average number of operations completed per second, we maximized this by reducing the number of times we called mem_sbrk. We also would check for space incremently (ie checking the previous then the next then both blocks) for space so that we would not have to check all three situations if the first one had enought space. We also tried to limit the amount of times we call the same operation and instead save the value in a variable. 


