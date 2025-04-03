//------------------------------------------
// NAME: Ahmed Abdelgalil / Zeko
// 
// REMARKS: The file contains the main function which is used to test the ObjectManager.c file.
//          The main function initializes the pool, inserts objects, retrieves objects, and drops references.
//          The main function then prints the retrieved object, dumps the pool, and destroys the pool. The main function returns 0.
//------------------------------------------
#include "ObjectManager.h"
#include <stdio.h>

int main(int argc, char *argv[]){
  char *ptr;
  int i;
  int id1,id2,id3;
  initPool();
  
  id1 = insertObject(100);
  printf("id1 = %d\n",id1);
  ptr = (char*)retrieveObject(id1);
  for (i = 0; i < 100; i++)
    ptr[i] = (char)(i%26 + 'A');
  
  printf("id1 = %d\n",id1);
  id2 = insertObject(4000);
  printf("id2 = %d\n",id2);
  id3 = insertObject(20200);
  printf("id3 = %d\n",id3);
  dropReference(id3);
  printf("id3 = %d\n",id3);
  id3 = insertObject(10);
  
  ptr = (char*)retrieveObject(id1);
  for (i = 0; i < 100; i++)
    fprintf(stderr,"%c",ptr[i]);
  fprintf(stderr,"\n");
  
  dumpPool();
  destroyPool();
  fprintf(stderr,"---\n");
  return 0;
}

