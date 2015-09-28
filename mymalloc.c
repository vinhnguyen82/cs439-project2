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
   // printf("header: %lu \n", (size_t) header);
   freeHeader = header;
   // printf("freeHeader: %lu \n", (size_t) freeHeader);

   *freeHeader = (chunk) {NULL, header, NULL, freeHeader, size, true};
}

void *my_malloc(size_t size)
{
   chunkptr iter = freeHeader;
   // printf("iter: %lu \n", (size_t) iter);

   while (iter != NULL) {
      // if free block is not enough

      if (iter->size < size + CHUNK_SIZE) {
         iter = iter->nextFreeChunk;
         // printf("pass");
         continue;
      }

      // remaining free space after use
      printf("iter->size: %lu\n", iter->size);
      size_t diffSize = iter->size - CHUNK_SIZE - size;
      printf("diffside: %lu\n", diffSize);

      if (diffSize <= CHUNK_SIZE) {
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
         printf("freeHeader: %lu \n", (size_t) freeHeader);
         printf("freeHeader point to null: %lu \n", (size_t) freeHeader->nextFreeChunk);
         // calculate the address of remain memory
         chunkptr remain = (chunkptr)((size_t) iter + CHUNK_SIZE + size);
         printf("remain: %lu \n", (size_t) remain);
         // printf("iter->prevFreeChunk: %lu \n", (size_t) iter->prevFreeChunk);
         // update current future used chunk
         iter->isfree = false;
         iter->size = size + CHUNK_SIZE;

         // update remaining free space
         remain->size = diffSize;
         printf("remain_diffsize: %lu \n", diffSize);
         printf("freeHeader: %lu \n", (size_t) freeHeader);
         printf("iter: %lu \n", (size_t) iter);
         remain->prevFreeChunk = iter->prevFreeChunk;
         remain->nextFreeChunk = iter->nextFreeChunk;
         (iter->prevFreeChunk)->nextFreeChunk = remain;

         printf("iter->nextFreeChunk: %lu \n", (size_t) iter->nextFreeChunk);
         remain->prev = iter;
         printf("iter->prevFreeChunk: %lu \n", (size_t) iter->prevFreeChunk);

         remain->next = iter->next;
         remain->isfree = true;

         // if free chunk is the first one, update freeHeader
         if (iter->prevFreeChunk == freeHeader) {
            freeHeader = remain;
            printf("freeHeader: %lu \n", (size_t) freeHeader);
            printf("header: %lu \n", (size_t) header);
            remain->prevFreeChunk = freeHeader;
         }

         // continue to update future used chunk
         iter->next = remain;
         iter->nextFreeChunk = NULL;
         iter->prevFreeChunk = NULL;
         // printf("Struct size %lu \n", CHUNK_SIZE);
         // printf("iter: %lu \n", (size_t) iter);
         // printf("Struct size %lu \n", CHUNK_SIZE);
         size_t total = (size_t) iter + CHUNK_SIZE;
         // printf("total %lu\n", total);
         // printf("Chunk header: %lu \n", (size_t) (iter + CHUNK_SIZE));

         // different when use (size_t) (iter + CHUNK_SIZE)
         return (void *)total;
      }
      printf("loop");
   }

   printf("no more memory");
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
// 
   // printf("return address: %lu \n", (size_t) ptr);
   // get the chunk's struct
   chunkptr temp = (chunkptr) ((size_t)ptr - CHUNK_SIZE);
   // it's already free
   if (temp->isfree == true) {
      return;
   }

   // printf("header: %lu \n", (size_t) header);
   // printf("temp: %lu \n", (size_t) temp);

   // change status of the chunk
   temp->isfree = true;


   // check if the freeHeader point to greater address
   if (freeHeader >= temp) {

      // rearrange pointers
      temp->nextFreeChunk = freeHeader;
      freeHeader->prevFreeChunk = temp;

      // point freeHeader to first free space
      freeHeader = temp;
      temp->prevFreeChunk = temp;
      printf("freeHeader after free: %lu \n", (size_t) freeHeader);
      printf("freeHeader->prevFreeChunk: %lu \n", (size_t) freeHeader->prevFreeChunk);


   } else {
   // if not, find the closed free spacse on the left
      chunkptr iter = temp->prev;
      // printf("iter: %lu \n", (size_t) iter);

      while (iter->isfree != true) {
         // printf("loop");
         iter = iter->prev;
      }

      // rearrage pointers
      temp->nextFreeChunk = iter->nextFreeChunk;
      if (temp->nextFreeChunk != NULL) {
         (iter->nextFreeChunk)->prevFreeChunk = temp;
      }
      temp->prevFreeChunk = iter;

   }
   

   // check if we can merge left or merge right
   // both left and right chunk are free
   if (temp != freeHeader && (temp->prev)->isfree == true && (temp->next)->isfree == true) {
      size_t total = (temp->prev)->size + temp->size + (temp->next)->size;
      (temp->prev)->nextFreeChunk = (temp->next)->nextFreeChunk;
      (temp->next)->prevFreeChunk = temp->prev;
      (temp->prev)->size = total;

      (temp->prev)->next = (temp->next)->next;
      (temp->next)->next = temp->prev;
   // only left chunk is free
   } else if (temp != freeHeader && (temp->prev)->isfree == true) {
      // printf("here");
      size_t total = (temp->prev)->size + temp->size;
      (temp->prev)->nextFreeChunk = temp->nextFreeChunk;
      (temp->nextFreeChunk)->prevFreeChunk = temp->prev;
      (temp->prev)->size = total;
      (temp->prev)->next = temp->next;
      (temp->next)->prev = temp->prev;
   // // only right chunk is free
   } else if ((temp->next)->isfree == true) {
      size_t total = temp->size + (temp->next)->size;
      temp->nextFreeChunk = (temp->next)->nextFreeChunk;
      (temp->next)->prevFreeChunk = temp;
      temp->size = total;
      temp->next = (temp->next)->next;
      if (temp->next != NULL) {
         (temp->next)->prev = temp;
      }
   }
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

   chunkptr iter = header;
   int count = 0;
   while (iter != NULL) {
      
      size_t size = iter->size / 1024 / 1024;
      if (iter->next == NULL) {
         draw_box(stream, size, iter->isfree, 1);
         return;
      }
      draw_box(stream, size, iter->isfree, 0);
      iter = iter->next;
   }
   // the following is an example
   // of a heap with four
   // blocks of sizes 
   // 1, 4, 1, and 5 
   // and the third block unallocated
   // (empty)
   // draw_box(stream, 1, 0, 0);
   // draw_box(stream, 4, 0, 0);
   // draw_box(stream, 1, 1, 0);
   // draw_box(stream, 5, 0, 1);
}

uint64_t my_address(void *ptr)
{
   return ((((uint64_t) ptr) - ((uint64_t) mem)) / MiB);
}
