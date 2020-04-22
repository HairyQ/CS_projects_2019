/*
 * Harrison Quick - u1098604
 * U of U - CS 4400
 * 11 - 18 - 19
 * 
 * I hate turning these in becuase I know this doesn't work as is..
 * 
 * But I think it could turn out to be a great design, and I want to
 * finish its implementation later. 
 *
 * What this design is supposed to do is take an approach I would call
 * "Perfect fit or least-snug fit". The idea here is to try to minimize
 * external fragmentation - Obviously, under this approach my allocator
 * would first try to find an available free block the perfect size for 
 * what it's trying to allocate for. I figured this would be worth the
 * implementation, because I assume any program is going to be allocating
 * and freeing at least several of the same type of object.
 *
 * When it doesn't find perfects, which it often won't, my allocator takes 
 * a kind of counterintuitive approach: it looks for the largest free 
 * block in the linked list of free blocks, and when it finds the largest
 * block, returns the pointer to that block. The idea behind this is that
 * it's better to have a small number of large free blocks than a large 
 * number of small free blocks, so as to minimize the amount of time it 
 * takes to find a free block when calling mm_malloc. 
 *
 * Although I don't have the time to implement it for this assigment, my
 * plan for coallescing was as follows:
 * I didn't want to coallesce every time I called free, because I read
 * that this would be expensive. Rather, I would opt for delayed 
 * coallescing, and only do so before I make a call for more pages,
 * hopefully often preventing the need to grab more pages.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* always use 16-byte alignment */
#define ALIGNMENT 16
#define HEADERSIZE 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* rounds up to the nearest multiple of mem_pagesize() */
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1))

// Given a payload pointer, get the header or footer pointer
#define HDRP(bp) ((char *)(bp) - HEADERSIZE)
#define FTRP(bp) ((char *)(bp)+GET_SIZE(HDRP(bp))-ALIGNMENT)

// Given a payload pointer, get the next or previous payload pointer
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-ALIGNMENT))


// ******These macros assume you are using a size_t for headers and footers ******
//Given a pointer to a header, get or set its value
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))

// Combine a size and alloc bit
#define PACK(size, alloc) ((size) | (alloc))

// Given a header pointer, get the alloc or size
#define GET_ALLOC(p) (GET(p) & 0x1)
#define PUT_ALLOC(p) (PUT(p, GET(p) | 0x1))
#define SET_ALLOC_NEXT(p) (PUT(p, GET(p) | 0x2))
#define SET_ALLOC_PREV(p) (PUT(p, GET(p) | 0x4))
#define GET_SIZE(p) (GET(p) & ~0xF)

typedef struct Node{
  struct Node *next;
  struct Node *prev;
} free_node, *free_node_ptr;

void *current_avail = NULL;
void *epilogue = NULL;
free_node *beginning = NULL; // Beginning of linked list
free_node *end = NULL; // End of linked list
free_node * current_node; // For implementation
int current_avail_size = 0;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  current_avail = NULL;
  epilogue = NULL;
  beginning = NULL; // Beginning of linked list
  end = NULL; // End of linked list
  current_avail_size = 0;
  
  mm_initializer(PAGE_ALIGN(mem_pagesize() + 48));
  
  return 0;
}

/* 
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)
{
  size_t newsize = ALIGN(size);
  size_t largest = 0;
  void * largest_free = NULL;
  void *p = NULL;
  
  if (end == NULL)
    {
       mm_initializer(PAGE_ALIGN(newsize + 48));
       mm_malloc(size);
    }
  else
   {
      current_node = beginning;
      while (current_node -> next)
	{
	  p = (void *)current_node - 8; // Go to header
	  if (GET_SIZE(p) == newsize) // Perfect fit found
	    {
	      set_allocated(p, newsize);
	      return p;
	    }
	  if (GET_SIZE(p) > largest) // Otherwise, is it largest fit?
	    {
	      if (GET_SIZE(p) > newsize)
		{
		  largest_free = p;
		  largest = GET_SIZE(p);
		}
	    }
	}
      p = (void *)current_node - 8; // One more time for final node just in case
      if (GET_SIZE(p) == newsize) // Perfect fit found
	  return set_allocated(p, newsize);
      if (GET_SIZE(p) > largest) // Largest fit?
	{
	  if (GET_SIZE(p) > newsize)
	    {
	      largest_free = p;
	      largest = GET_SIZE(p);
	    }
	}

      //////////////////////////
      //By this point, we have either found a perfect fit, or we have found the largest space
      //to put the block. This is to ensure a small number of large free blocks as opposed to
      //a large number of small free blocks, to avoid severe external fragmentation. From here,
      //if perfect fit was found, we have returned. So put in largest spot. If nothing was
      //found, we didn't find a free space with enough room, so we ask for more pages...
      //////////////////////////

      if (largest_free) // Large space found
	{
	  return set_allocated(split_block(largest, newsize, largest_free), newsize);
	}
      else              // Otherwise, we need more pages
	{
	  mm_initializer(PAGE_ALIGN(newsize + 48));
	  return mm_malloc(newsize);
	}
       }

  return p;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  free_node_ptr nodeptr = ptr;
  end -> next = nodeptr;
  nodeptr -> prev = end;

  end = nodeptr;
  
  //ptr -= 8;
  //size_t size = *(size_t *)ptr & ~0xF;
  
}

void * split_block(size_t oldsize, size_t newsize, void *p)
{
  //fflush(stdout);
  //p = HDRP(p);     // Jump to header
  void *retHeader = p;
  int is_beg, ending = 0;
  PUT(p, newsize); // Put new size in header
  p += 8;          // Jump to block (currently should be free)

  free_node tempNode = *(free_node_ptr)p; // Save free list pointer info
  if (beginning == (free_node_ptr)p)
    is_beg++;
  if (end == (free_node_ptr)p)
    ending++;

  p += newsize; // Jump to new footer location
  PUT(p, newsize);   // Store size info here, too
  p += 8;            // Jump to new header location

  PUT(p, oldsize - newsize - 16); // Store new size in new header
  p += 8;                           // Jump to new block
  *(free_node *)p = tempNode;       // Store the free list!
  if (is_beg)
    beginning = (free_node *)p;
  if (ending)
    end = (free_node *)p;
  
  p += oldsize - newsize - 16;     // Jump to footer to revise
  PUT(p, oldsize - newsize - 16); // Store new size in footer

  //Blocks are now split - return old header value

  return retHeader;
}

void mm_initializer(size_t size)
{
  current_avail = mem_map(PAGE_ALIGN(size));
  current_avail_size = size - 48;

  current_avail += 8; // Padding for 8-byte double-word boundary

  size_t s = (2 << 3) + 1;//+1 because "allocated" - this is for prologue
  
  *(size_t *)current_avail = s;
  current_avail += 8;

  s = 17;

  *(size_t *)current_avail = s; // End of prologue
  current_avail += 8; 

  s = current_avail_size;
  
  *(size_t *)current_avail = s; // Beginning of newly initialized free pg header
  current_avail += 8; // Beginning of free block

  if (beginning == NULL && end == NULL)
    {
      beginning = end = (free_node *)current_avail; //Beginning of free list
      beginning -> next = NULL;
      beginning -> prev = NULL;  
    }
  
  else if (end != NULL)
  {
    free_node currNode = *(free_node *)current_avail;
    currNode.prev = end -> prev;
    currNode.next = end;
    end -> prev = current_avail;
  }
      
  current_avail += current_avail_size;

  *(size_t *)current_avail = s; // Footer for rest of available heap
  current_avail += 8; // Epilogue
  epilogue = current_avail;

  s = 1;

  *(size_t *)current_avail =  1; // represents 0/1 for epilogue
}

/* Set a block to allocated
*  Update block headers/footers as needed
*  Update free list if applicable
*  Split block if applicable
*/
void * set_allocated(void *p, size_t newsize)
{
  PUT_ALLOC(p); // & 0x1 for "allocated"
  p -= 8; // Go to footer of previous node
  if (*(size_t *)p > 17) // Check to see if at prologue
    SET_ALLOC_NEXT(p);
  //Otherwise, set prev's "next" bit
  
  p += newsize + 16; // Go to footer of current block
  PUT_ALLOC(p); // & 0x1 for "allocated"
  
  p += 8; // Go to header of next block
  if (*(size_t *)p != 1) // Check for epilogue
    SET_ALLOC_PREV(p); // & 0x4 for "allocated" otherwise

  p -= (newsize + 8); // Reset p
  
  // Now we need to reset pointers on free list
  if (current_node -> prev == NULL)           // Case: this is beginning of list
    {
      if (current_node -> next != NULL)
	{
	  beginning = current_node -> next;
	  current_node -> next -> prev = NULL;
	}
      else if (beginning == end && beginning == current_node)//Taking up all space - need new page when we realloc
	beginning = end = NULL;
    }
  else if (current_node -> next == NULL)     // Case: this is end of list
    {
      end = current_node -> prev;
      current_node -> prev -> next = NULL;
    }
  else                                       // Case: middle node in list
    {
      current_node -> prev -> next = current_node -> next;
      current_node -> next -> prev = current_node -> prev;
    }

  return p;
}

/* Request more memory by calling mem_map
*  Initialize the new chunk of memory as applicable
*  Update free list if applicable
*/
void extend(size_t s)
{
  return;
}

/* Coalesce a free block if applicable
*  Returns pointer to new coalesced block
*/
void* coalesce(void *bp)
{
  return NULL;
}
