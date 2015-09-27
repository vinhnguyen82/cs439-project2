#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "mymalloc.h"

static void *mem;

typedef int bool;
#define true 1
#define false 0

typedef struct chunk {
   struct chunk *next;
   struct chunk *prev;
   struct chunk *nextFreeChunk;
   size_t size;
   unsigned status;
}

chunk *header = NULL;
chuck *freeHeader = NULL;

// typedef struct freeChunk {
//    struct chunk *c;
//    size
// }

void my_malloc_init(size_t size)
{
   mem = malloc(size);
   chunk initial = {
      .next = NULL;
      .prev = NULL;
      .nextFreeChunk = NULL;
      .size = size;
      status = 0;

   }

   header = &initial;
   freeHeader = &initial;
}

// go from temp = freeHeader
// if temp is enough change stop = true
void *my_malloc(size_t size)
{
   bool stop = false;
   chuck *temp = freeHeader;
   while (freeHeader->nextFreeChunk != NULL && !stop) {
      if (temp->nextFreeChunk.size = size) {
         temp->nextFreeChunk = temp->nextFreeChunk.nextFreeChunk;
         temp.status = 1;
      } else if (temp->nextFreeChunk.size > size) {
         size_t newFreeSize = temp->nextFreeChunk.size - size;

      }

      }
   }
   return NULL;
}

void updateFreeChunkOrder(chunk *ptr) {
   
}


/**
 * @brief Free a previously allocated block so that it
 *        becomes available again.
 *        If possible, coalesc the block with adjacent free
 *        blocks.
 * @param [in] ptr   the pointer to the block.
 */
void my_free(void *ptr)
{
   // TODO: implement
}

static void draw_box(FILE *stream, int size, int empty, int last)
{
   int i;
   int pad = size / 2;

   fprintf(stream, "+---------------------+\n");

   if (!empty) fprintf(stream, "%c[%d;%dm", 0x1B, 7, 37);

   for (i=0; i<pad; i++)
   {
      fprintf(stream, "|                     |\n");
   }

   fprintf(stream, "|         %2d          |\n", size);

   for (i++; i<size; i++)
   {
      fprintf(stream, "|                     |\n");   
   }

   if (!empty) fprintf(stream, "%c[%dm", 0x1B, 0);

   if (last) fprintf(stream, "+---------------------+\n");
}

void my_dump_mem(FILE *stream)
{
   // TODO: implement


   // the following is an example
   // of a heap with four
   // blocks of sizes 
   // 1, 4, 1, and 5 
   // and the third block unallocated
   // (empty)
   draw_box(stream, 1, 0, 0);
   draw_box(stream, 4, 0, 0);
   draw_box(stream, 1, 1, 0);
   draw_box(stream, 5, 0, 1);
}

uint64_t my_address(void *ptr)
{
   return ((((uint64_t) ptr) - ((uint64_t) mem)) / MiB);
}
