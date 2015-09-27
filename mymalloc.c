#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "mymalloc.h"

static void *mem;

typedef int bool;
#define true 1
#define false 0

typedef struct chunk* chunkptr;

typedef struct chunk {
   chunkptr next;
   chunkptr prev;
   chunkptr nextFreeChunk;
   chunkptr prevFreeChunk;
   size_t size;
   unsigned isfree;
} chunk;

chunkptr header;
chunkptr freeHeader;
const size_t CHUNK_SIZE= sizeof(chunk);

void my_malloc_init(size_t size)
{
   mem = malloc(size);
   header = (chunkptr) mem;
   freeHeader = header;

   *freeHeader = (chunk) {NULL, header, NULL, freeHeader, size, true};
}

void *my_malloc(size_t size)
{
   chunkptr iter = freeHeader;
   printf("Iter: %lu \n", (size_t) iter);

   while (iter != NULL) {
      // if free block is not enough
      if (iter->size < size + CHUNK_SIZE) {
         iter = iter->nextFreeChunk;
         continue;
      }

      // remaining free space after use
      size_t diffSize = iter->size - CHUNK_SIZE - size;

      if (diffSize <= CHUNK_SIZE) {
         printf("inside of <=\n");

         // we will give them all the free block
         // update current free chunk to allocated
         iter->isfree = false;

         // check if it is the last block
         if (iter->nextFreeChunk != NULL) {
            (iter->nextFreeChunk)->nextFreeChunk = iter->prevFreeChunk;
         }

         // if free chunk is the first one, update freeHeader
         if (iter->prevFreeChunk == freeHeader) {
            freeHeader = iter->nextFreeChunk;
         }

         // return the pointer at address after struct of that location
         return (iter + CHUNK_SIZE);

      } else {
         printf("inside of >\n");

         // calculate the address of remain memory
         chunkptr remain = (chunkptr)((size_t) iter + CHUNK_SIZE + size);
         printf("remain: %lu \n", (size_t) remain);

         // update current future used chunk
         iter->isfree = false;
         iter->size = size + CHUNK_SIZE;

         // update remaining free space
         remain->size = diffSize;
         remain->prevFreeChunk = iter->prevFreeChunk;
         printf("Prev free: %lu \n", (size_t) remain->prevFreeChunk);
         remain->nextFreeChunk = iter->nextFreeChunk;
         remain->prev = iter;
         remain->next = iter->next;
         remain->isfree = true;

         // if free chunk is the first one, update freeHeader
         if (iter->prevFreeChunk == freeHeader) {
            freeHeader = remain;
            remain->prevFreeChunk = freeHeader;
         }

         // continue to update future used chunk
         iter->next = remain;
         iter->nextFreeChunk = NULL;
         iter->prevFreeChunk = NULL;

         return (iter + CHUNK_SIZE);
      }
   }
   return NULL;
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
