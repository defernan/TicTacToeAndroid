// REQUIRED-5: A series of different-size allocations should work correctly.

#include <stdlib.h>

#include "../test.h"
#include "../../kheap.h"

#define SPACE_SIZE_INITIAL  (5 * 1024 * 1024)   // 5MiB
#define SPACE_SIZE_TOTAL    (10 * 1024 * 1024)  // 10MiB

#define ALLOCATION_SIZE_S   20
#define ALLOCATION_SIZE   200

int main(int argc, char **argv)
{
   // allocate space for kmalloc to play with
   void *space = malloc(SPACE_SIZE_TOTAL);

   // create the heap
   struct heap *heap = heap_create(space,
                                   space + SPACE_SIZE_INITIAL,
                                   space + SPACE_SIZE_TOTAL);

   // allocate three times
   void *allocated1 = kalloc_heap(ALLOCATION_SIZE, 0, heap);
   void *allocated2 = kalloc_heap(ALLOCATION_SIZE, 0, heap);
   void *allocated3 = kalloc_heap(ALLOCATION_SIZE, 0, heap);

   size_t total_size = ALLOCATION_SIZE +
                             sizeof(struct header) +
                             sizeof(struct footer);

   // get the header and footer of each allocation
   struct header *head1 = allocated1 - sizeof(struct header);
   struct header *head2 = allocated2 - sizeof(struct header);
   struct header *head3 = allocated3 - sizeof(struct header);

   // the allocations should be next to each other in memory like so:
   // +-----------------------------------------------------------------------+
   // | head1 | data1 | foot1 | head2 | data2 | foot2 | head3 | data3 | foot3 |
   // +-----------------------------------------------------------------------+

   // free second allocation
   kfree_heap(allocated2, heap);

   // test to make sure that the second block is not allocated
   t_assert("The header should indicate that the first block is allocated",
            head1->allocated == 1);
   t_assert("The header should indicate that the second block is not allocated",
            head2->allocated == 0);
   t_assert("The header should indicate that the third block is allocated",
            head3->allocated == 1);
   // free first allocation
   
   kfree_heap(allocated1, heap);

   // check to see that the first and second blocks are combined
   t_assert("The header should indicate that the first block is allocated",
            head1->allocated == 0);
   t_assert("The size of the first block should now be the size of the first plus the second",
            head1->size == total_size*2);
   t_assert("The header should indicate that the third block is allocated",
            head3->allocated == 1);

   // free the heap space
   free(space);

   return 0;
}
