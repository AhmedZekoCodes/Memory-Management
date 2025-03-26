// This file contains the implementation of the ObjectManager
#include "ObjectManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>


// You can add other header files if needed
bool insertInList(int size);

// tracks the next reference (ID) to use, we start at 1 so we can use 0 as the NULL reference
static Ref nextRef = 1;

// A Memblock holds the relevent information associated with an allocated block of memory by our memory manager
typedef struct MEMBLOCK MemBlock;


// information needed to track our objects in memory
struct MEMBLOCK
{
  int numBytes;    // how big is this object?
  int startAddr;   // where the object starts
  Ref ref;         // the reference used to identify the object
  int count;       // the number of references to this object
  MemBlock *next;  // pointer to next block.  Blocks stored in a linked list.
};


// The blocks are stored in a linked list where the start of the list is pointed to by memBlockStart.
static MemBlock *memBlockStart; // start of linked list of blocks allocated
static MemBlock *memBlockEnd;   // end of linked list of blocks allocated
static int numBlocks;            // number of blocks allocated

// our buffers.  This is where we allocate memory from.  One of these is always the current buffer.  The other is used for swapping
//during compaction stage.

static unsigned char buffer1[MEMORY_SIZE];
static unsigned char buffer2[MEMORY_SIZE];

// points to the current buffer containing our data
static unsigned char *currBuffer = buffer1;

// points to the location of the next available memory location
static int freeIndex = 0;

// performs required setup
void initPool()
{
  //write your code here
  memBlockStart = NULL;
  memBlockEnd = NULL;
  numBlocks = 0;
}

// performs required clean up
void destroyPool()
{
  //write your code here
  //we want to delete all nodes from the linked list.
  MemBlock* prev = NULL;
  MemBlock* curr = memBlockStart;
  // Traverse the linked list and free each node
  while (curr){
    prev = curr;
    curr = curr->next;
    free(prev);
  }
  // Reset the memory manager
  memBlockStart = NULL;
  memBlockEnd = NULL;
}

// Adds the given object into our buffer. It will fire the garbage collector as required.
// We always assume that an insert always creates a new object...
// On success it returns the reference number for the block allocated.
// On failure it returns NULL_REF (0)
Ref insertObject( const int size )
{
  printf("[INSERTING OBJECT]\n");
  //write your code here
  if (size <= 0){
    return NULL_REF;
  }

  Ref reference = NULL_REF;

  // Check if we have enough memory to allocate the object
  // If not, we need to compact the memory
  if (freeIndex + size > MEMORY_SIZE){
    // Garbage collection
    compact();
  }

  // If we have enough memory to allocate the object
  // We need to insert the object into the list
  if (freeIndex + size <  MEMORY_SIZE){
    // Insert the object into the list
    insertInList(size);
    // Set the reference to the reference of the last block in the list
    reference = memBlockEnd->ref;
    // Update the start and end addresses
    memBlockEnd->startAddr = freeIndex;
    printf("Start Address: %d\n", memBlockEnd->startAddr);
    freeIndex += size;
    printf("End Address: %d\n", freeIndex);
  }
  return reference;
}

// returns a pointer to the object being requested
void *retrieveObject( const Ref ref )
{
  printf("[RETRIEVING OBJECT]\n");
  //write your code here
  // Traverse the linked list and return the object with the given reference
  MemBlock *curr = memBlockStart;
  while (curr){
    if (curr->ref == ref){
      // Return the offset of the object in the buffer
      return (void*) (buffer1 + (curr->startAddr));
    }
    curr = curr->next;
  }
  return NULL_REF;
}

// update our index to indicate that we have another reference to the given object
void addReference( const Ref ref )
{
  printf("[ADDING REFERENCE]\n");  
  //write your code here
  // Traverse the linked list and increment the count of the object with the given reference
  MemBlock *curr = memBlockStart;
  while (curr){
    if (curr->ref == ref){
      curr->count++;
      break;
    }
    curr = curr->next;
  }
}

// update our index to indicate that a reference is gone
void dropReference( const Ref ref )
{
  printf("[DROPPING REFERENCE]\n");
  //write your code here
  MemBlock *prev = NULL;
  MemBlock *curr = memBlockStart;
  while (curr){
    if (curr->ref == ref){
      curr->count--;
      if (curr->count == 0){
        // If head of list is equal to the current node, we need to update the head of the list
        if (memBlockStart == curr){
          memBlockStart = curr->next;
        }
        // Else we need to update the previous node's next pointer
        else {
          prev->next = curr->next;
        }
        // If end of list is equal to the current node, we need to update the end of the list
        if (memBlockEnd == curr){
          memBlockEnd = prev;
        }
        free(curr);
      }
      break;
    }
    prev = curr;
    curr = curr->next;
  }
}

// performs our garbage collection
void compact()
{
  //write your code here
  printf("[COMPACTING MEMORY]\n");
  MemBlock *curr = memBlockStart;
  int offset = 0;
  int totalObjects = 0;
  int currentBytesInUse = freeIndex;
  // Traverse the linked list and move the objects to the left
  while (curr){
    if (curr->count > 0){
      for (int i = 0; i < curr->numBytes; i++){
        currBuffer[offset + i] = currBuffer[curr->startAddr + i];
      }
      curr->startAddr = offset;
      offset += curr->numBytes;
    }
    totalObjects++;
    curr = curr->next;
  }
  fprintf(stdout, "Total Objects that exists: %d\n", totalObjects);
  fprintf(stdout, "Total Bytes Recovered: %d\n", currentBytesInUse - freeIndex);
  fprintf(stdout, "Current Bytes in Use: %d\n", freeIndex);
}

void dumpPool()
{
  printf("[DUMPING POOL]\n");
  //write your code here
  MemBlock *curr = memBlockStart;
  while (curr){
    fprintf(stdout, "Ref: %ld, Start Address: %d, Number of Bytes: %d, Count: %d\n", curr->ref, curr->startAddr, curr->numBytes, curr->count);
    curr = curr->next;
  }
}

//you may add additional function if needed
bool insertInList(int size)
{
  printf("[INSERTING IN LIST]\n");
  //write your code here
  MemBlock *newBlock = (MemBlock *)malloc(sizeof(MemBlock));
  #ifndef DEBUG // Check if debug is not defined
  if (newBlock == NULL){
    printf("Failed to allocate memory for new block\n");
    return false;
  }
  #elif
    assert(newBlock != NULL);
  #endif
  newBlock->ref = nextRef++;
  newBlock->numBytes = size;
  newBlock->startAddr = freeIndex;
  newBlock->count = 1;
  newBlock->next = NULL;
  // Check if end of list is not equal to NULL ~ if it is not, then we need to add the new block to the end of the list
  if (memBlockEnd != NULL){
    memBlockEnd->next = newBlock;
  }
  // Check if start of list is equal to NULL ~ if it is, then we need to set the start of the list to the new block
  if (!memBlockStart){
    memBlockStart = newBlock;
  }
  memBlockEnd = newBlock;
}