#include <stdio.h>

extern int mm_init (void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);
static void mm_initializer(size_t size);
static void* set_allocated(void *b, size_t size);
static void extend(size_t s);
static void* coalesce(void *bp);
static void* split_block(size_t size, size_t newsize, void *b);
