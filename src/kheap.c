// Implementation of the heap

#include "kheap.h"

#include "common.h"
/*******************************************************************
/ Things believed to be complete
/ memset -> YES
/ add_hole -> YES - coalescing
/ find_smallesthole-> YES
/ kalloc_heap ->YES
/ kfree_heap ->Yes
/ page allign in kalloc_heap -> Yes
/ colescing -> NO
/ ******************************************************************/

/*******************************************************************
/ POTENTIAL ERRORS
/ There is a lot of commented out code, this will be removed, left it in so that you can
/ follow the changes and logic more easily
/
/ ******************************************************************/
// headers for local functions
void *align(void *p);
ssize_t find_smallest_hole(size_t size,
                           u8int page_align,
                           struct heap *heap)
                           WARN_UNUSED;
s8int header_less_than(void *a, void *b);
s8int heap_resize(size_t new_size, struct heap *heap) WARN_UNUSED;
void add_hole(void *start, void *end, struct heap *heap);

// returns an aligned pointer
// if the address is not aligned, then the aligned address prior to the given
// address is returned
void *align(void *p)
{
   union
   {
      void *pointer;
      size_t integer;
   } u;

   u.pointer = p;
   u.integer &= PAGE_MASK;

   return u.pointer;
}

// less than function for comparing the sizes of memory chunks, using the
// headers; a < b iff a's size is less than b's size
// a and b should both be pointers to header structs
s8int header_less_than(void *a, void *b)
{
   return (((struct header*)a)->size < ((struct header*)b)->size) ? 1 : 0;
}

// creates a heap at the given start address, end address, and maximum growth
// size
// start, end, and max should all be page-aligned (but if they aren't, we just
// waste some space)
struct heap *heap_create(void *start,
                         void *end,
                         void *max)
{
   // in the real kernel, we would kmalloc here, where kmalloc is just a
   // placement implementation, since the heap does not exist yet!
   // for the userspace projet, we can assume that the memory has already been
   // allocated
   //
   // the memory layout from start to end is as follows:
   // | heap struct | free list | actual data |
   //struct heap *heap = (struct heap*)kmalloc(sizeof(heap_t));
   struct heap *heap = (struct heap*)start;

   // create the free list
   heap->free_list = sorted_array_place((void *)start +
                                           sizeof(struct sorted_array),
                                     HEAP_FREE_LIST_SIZE,
                                     &header_less_than);

   // move the start address of the heap, to reflect where data can be place,
   // now that the free list is in the initial portion of the heap's memory
   // address space
   start += sizeof(struct sorted_array) + sizeof(void *) * HEAP_FREE_LIST_SIZE;

   // make sure the start address is page-aligned
   if(align(start) != start) {
      start = align(start) + PAGE_SIZE;
   }

   // write the avariables into the heap structure
   heap->start_address = start;
   heap->end_address = end;
   heap->max_address = max;

   // start with a large hole the size of the memory region
   add_hole(start, end, heap);

   return heap;
}

// expands or contracts the heap to the new_size
// returns a negative value on error, 0 on success
s8int heap_resize(size_t new_size, struct heap *heap)
{
   // make sure the new end address is page-aligned
   // since the heap starts on a page boundary, we need only align the size
   if(align((void *)new_size) != (void *)new_size) {
      new_size = (size_t)(align((void *)new_size)) + PAGE_SIZE;
   }

   if(new_size < heap->end_address - heap->start_address)
   {
      // contracting the heap

      // we are going to naively assume that the heap is not being resized to
      // a value that is too small

      // paging code would go here to free pages
      // for now, just assume that in our flat memory space, memory is
      // available and does not need to be allocated or freed
   }
   else if(new_size > heap->end_address - heap->start_address)
   {
      // expanding the heap

      // make sure the new size is within the bounds
      if(heap->start_address + new_size > heap->max_address) {
         // not within bounds
         return -1;
      }

      // paging code would be here to allocate pages
      // for now, just assume that in our flat memory space, memory is
      // available and does not need to be allocated or freed
   }
   else
   {
      // same size - do nothing
   }

   // set the size of the heap
   heap->end_address = heap->start_address + new_size;

   return 0;
}

// find the smallest hole that will fit the requested size
// if a hole is found, the index in the heap free list is returned
// if a hole is not found, then -1 is returned
// size must include the size of the header and footer, in addition to the
// size that the actual users wishes to request
ssize_t find_smallest_hole(size_t size,
                           u8int page_align,
                           struct heap *heap)
{
   // TODO: IMPLEMENT THIS FUNCTION

   // pseudocode:
   // 1: iterate over free list
   // 2: see if the chunk is large enough for the requested size - remember to
   //    include page alignment!
   // 3: if large enough, return
   // 4: if not large enough, continue to the next chunk in the iteration
   // 5: if the end is reached before a chunk is found, return -1
  
   //1 iterate
   ssize_t i;
   for(i = 0; i < heap->free_list.size; i++){
        struct header *hole = (struct header*)sorted_array_lookup(i, &heap->free_list);
        //2
        //check if kalloc calls page align
        if( page_align > 0 ){
            //page align
            ssize_t loc = (ssize_t) hole;
            s32int offset = 0;
            //orig
            //(loc+sizeof(header) ) & 0xFFFFF000
            // suggested? loc & PAGE_SIZE != loc
            if( align( loc + sizeof(struct header)) != (loc + sizeof(struct header)) ){
            // if( align( (void*)(loc + sizeof(struct header) ) ) != (loc + sizeof(struct header)) ){
                //align
                offset = PAGE_SIZE - (loc + sizeof(struct header))%PAGE_SIZE;
            }
            s32int holeSize = (s32int)hole->size - offset;
            //3
            //check for fit
            if(holeSize >= size){
                return i;
            }
        }else if(hole->size >= size){
            //3
            return i;
        }
        //4 iterate
   }
   //5 end reached
   return -1;
}

// creates and writes a hole that spans [start,end)
void add_hole(void *start, void *end, struct heap *heap)
{
   // TODO: IMPLEMENT THIS FUNCTION
   // pseudocode:
   // 0. determine if coalesing is possible; if so, remove the appropriate
   //    holes on either side of start and end, and then call add_hole
   //    recursively on the larger region (can skip if no coalesing)
   // 1. write header and footer to memory
   // 2. add chunk to free list
  
   //0

   //1 write header and footer to memory 
   struct header *hole = (struct header*) start;
   hole->size = (ssize_t)end - (ssize_t)start; 
   hole->magic = HEAP_MAGIC; 
   hole->allocated = 0;

   struct footer* footer = (struct footer*)( (ssize_t)end - sizeof( struct footer) );
   if ((ssize_t)footer < heap->end_address) {
      footer->header = hole;
      footer->magic = HEAP_MAGIC;
   }
   
   //2 add chunk to free list 
   sorted_array_insert((void*)hole, &heap->free_list );
}

void *kalloc_heap(size_t size, u8int page_align, struct heap *heap)
{
   // TODO: IMPLEMENT THIS FUNCTION

   // pseudocode:
   // 1. figure out size of needed free list entry (add size of header and
   //    footer with sizeof(struct header) / sizeof(struct footer))
   // 2. find a hole to allocate using find_smallest_hole
   // 3. if no hole found, resize the heap, and then start again at 2
   // 4. remove the found hole from the free list to use for allocation
   // 5. page-align, if necessary
   // 5.1. determine if page-alignment makes a good hole before our
   //      allocation; if so, add that hole
   // 6. mark the chunk as allocated, write the header/footer (if necessary)
   // 7. return pointer to allocated portion of memory

   // 1 figure out size of entry
   u32int new_size = (u32int) size + sizeof(struct header) + sizeof(struct footer);
   // 2 find a hole
   ssize_t iterator = (ssize_t) find_smallest_hole(new_size, page_align, heap);
   // 3 if no hole, resize heap
   if (iterator == -1) {
      u32int old_heap_length = heap->end_address - heap->start_address;
      u32int old_end_address = heap->end_address; 
      heap_resize(old_heap_length+new_size, heap);
      u32int new_heap_length = heap->end_address - heap->start_address;

      ssize_t f_list = -1;
      u32int value = 0;

      for (iterator = 0; iterator < heap->free_list.size; iterator++) {
         u32int temp_value = (u32int) sorted_array_lookup(iterator, &heap->free_list);
         if (temp_value > value) {
            value = temp_value;
            f_list = iterator;
         }
      }

      if (f_list == -1) {
         add_hole(old_end_address, old_end_address + new_heap_length - old_heap_length, heap);
      }
      else {
         struct header *head = sorted_array_lookup(f_list, &heap->free_list);
         head->size += new_heap_length - old_heap_length;
         struct footer *foot = (struct footer *) ((u32int)head + head->size - sizeof(struct footer));
         foot->header = head;
         foot->magic = HEAP_MAGIC;
      }
      return kalloc_heap(size, page_align, heap);
   }
   struct header *old_hole_head = (struct header *)sorted_array_lookup((ssize_t)iterator, &heap->free_list);
   ssize_t old_hole_pos = (ssize_t)old_hole_head;
   u32int old_hole_size = (u32int)old_hole_head->size;
   if (old_hole_size - new_size < sizeof(struct header) + sizeof(struct footer)) {
      size += old_hole_size - new_size;
      new_size = old_hole_size;
   }
   // 5 page align
   // double check this, not completely sure here
   if (page_align > 0 && (align(old_hole_pos + sizeof(struct header)) != old_hole_pos + sizeof(struct header)) ) {
      old_hole_pos = align(old_hole_pos + sizeof(struct header)) - sizeof(struct header);
   }
   else {
      // 4
      sorted_array_remove((ssize_t)iterator, &heap->free_list);
   }
   // 6
   struct header *block = old_hole_pos;
   block->magic = HEAP_MAGIC;
   block->allocated = 1;
   block->size = new_size;
   
   struct footer *block_foot = (struct footer *) ((ssize_t)block + new_size - sizeof(struct footer));
   block_foot->magic = HEAP_MAGIC;
   block_foot->header = block;
   
   // not sure if the following is necessary for project
   // adds a new hole if the original hole was split
   if (old_hole_size - new_size > 0) {
      ssize_t add_hole_start = (ssize_t)(old_hole_pos + new_size);
      ssize_t add_hole_end = (ssize_t)(add_hole_start + old_hole_size - new_size);
      add_hole(add_hole_start, add_hole_end, heap);
   }

   return (void *) ((ssize_t)block + sizeof(struct header));
}

void kfree_heap(void *p, struct heap *heap)
{
   // TODO: IMPLEMENT THIS FUNCTION

   // pseudocode:
   // 1. check for a null pointer before proceeding
   // 2. get the header and the footer based on the passed pointer
   // 3. mark the chunk as free in the header
   // 4. add a hole in the space that was previously allocated

   // 1 check for null pointer
   if (p == 0) {
      return;
   }
   // 2 header and footer
   struct header *hole = (struct header*) ((ssize_t)p - sizeof(struct header));
   struct footer *hole_foot = (struct footer*) ((ssize_t)hole + hole->size - sizeof(struct footer));
   // 3 set allocated to 0 in hole
   hole->allocated = 0;
   // 4 add hole to free list
   sorted_array_insert((void*)hole, &heap->free_list);
}
