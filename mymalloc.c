#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "mymalloc.h"

static void *mem;

typedef int bool;
#define true 1
#define false 0

// #define DEBUG

typedef struct chunk* chunkptr;

typedef struct chunk {
   chunkptr next;
   chunkptr prev;
   chunkptr nextFreeChunk;
   chunkptr prevFreeChunk;
   size_t size;
   unsigned isfree;
   void* location;
} chunk;

chunkptr header = NULL;
chunkptr freeHeader = NULL;

void my_malloc_init(size_t size)
{
   mem = malloc(size);
   // initial first chunk
   header = malloc (sizeof(chunk));
   freeHeader = header;
   *header = (struct chunk) {NULL, header, NULL, freeHeader, size, true, mem};
}

void *my_malloc(size_t size)
{  
   chunkptr iter = freeHeader;
   while (iter != NULL) {
      // if free block is not enough
      if (iter->size < size) {
         iter = iter->nextFreeChunk;
         continue;
      }

      // remaining free space after use
      size_t diffSize = iter->size - size;

      if (diffSize == 0) {
         // we will give them all the free block
         // update current free chunk to allocated
         iter->isfree = false;

         // if free chunk is the first one, update freeHeader
         if (iter->prevFreeChunk == freeHeader) {
            freeHeader = iter->nextFreeChunk;
            freeHeader->prevFreeChunk = freeHeader;
         } else {
            (iter->prevFreeChunk)->nextFreeChunk = iter->nextFreeChunk;
         }

         // delete iter info about free chunk
         iter->prevFreeChunk = NULL;
         iter->nextFreeChunk = NULL;
         return iter->location;

      } else {
         // calculate the address of remain memory
         size_t loc = (size_t) iter->location + size;

         // create a chunk for remaining space
         chunkptr remain = malloc(sizeof(chunk));
         *remain = (chunk){iter->next, iter, iter->nextFreeChunk, iter->prevFreeChunk, iter->size - size, true, (void *) loc};

         if (iter->prevFreeChunk == freeHeader) {
            remain->prevFreeChunk = remain;
            freeHeader = remain;
         }

         // udpate pointers
         // iter is not the last chunk
         if (iter->next != NULL) {
            (iter->next)->prev = remain;
         }
         iter->size = size;
         iter->isfree = false;
         iter->next = remain;

         // reset iter free status
         iter->nextFreeChunk = NULL;
         iter->prevFreeChunk = NULL;

         return iter->location;
      }
   }
   return NULL;
}

void my_free(void *ptr)
{
   // get the chunk's struct
   // find struct of this location
   chunkptr iter = header;
   while (iter->location != ptr) {
      iter = iter->next;
   }
   chunkptr current = iter;
   // it's already free
   if (current->isfree == true) {
      return;
   }

   // change status of the chunk
   current->isfree = true;

   // check if the freeHeader point to greater address
   if (freeHeader->location >= current->location) {

      // rearrange pointers
      current->nextFreeChunk = freeHeader;
      freeHeader->prevFreeChunk = current;

      // point freeHeader to first free space
      freeHeader = current;
      current->prevFreeChunk = current;

   } else {
      // if not, find the closed free spacse on the left
      chunkptr iter = current->prev;

      while (iter->isfree != true) {
         iter = iter->prev;
      }

      // rearrage pointers
      current->nextFreeChunk = iter->nextFreeChunk;
      if (current->nextFreeChunk != NULL) {
         (iter->nextFreeChunk)->prevFreeChunk = current;
      }
      current->prevFreeChunk = iter;
      iter->nextFreeChunk = current;

   }

   // check if we can merge left or merge right
   // both left and right chunk are free
   chunkptr i = freeHeader; 
   while (i != NULL) {
      chunkptr temp = i->next;
      // if i is last chunk
      if (temp == NULL) {
         break;
      }

      // next one is a free chunk
      if (temp->isfree == true) {
         // merge
         i->size += temp->size;

         chunkptr tempNext = temp->next;
         i->next = tempNext;

         if (tempNext != NULL) {
            tempNext->prev = i;
         }

         i->nextFreeChunk = temp->nextFreeChunk;
         if (temp->nextFreeChunk != NULL) {
            (i->nextFreeChunk)->prevFreeChunk = temp;
         }
         free(temp);
      } else {
         // if not, go to the next free chunk
         i = i->nextFreeChunk;
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
   #ifdef DEBUG
   printf("header: %lu \n", (size_t) header);
   chunkptr i = header;
   int count = 0;
   while (i != NULL) {
      ++count;
      printf("count = %d\n", count);
      printf("freeHeader: %lu \n", (size_t) freeHeader);
      printf("current: %lu \n", (size_t) i);
      printf("current location: %lu \n", (size_t) i->location);

      printf("prev: %lu \n", (size_t) i->prev);
      printf("next: %lu \n", (size_t) i->next);
      printf("prevFree: %lu \n", (size_t) i->prevFreeChunk);
      printf("nextFree: %lu \n", (size_t) i->nextFreeChunk);
      printf("size: %lu \n", i->size);
      printf("isfree: %u\n", i->isfree);
      printf("--------------------\n");

      i = i->next;
   }
   #endif

   chunkptr iter = header;
   while (iter != NULL) {
      // assume allocated blocks are MiB
      size_t size = iter->size / MiB;
      if (iter->next == NULL) {
         draw_box(stream, size, iter->isfree, 1);
         return;
      }
      draw_box(stream, size, iter->isfree, 0);
      iter = iter->next;
   }
}

uint64_t my_address(void *ptr)
{
   return ((((uint64_t) ptr) - ((uint64_t) mem)) / MiB);
}
