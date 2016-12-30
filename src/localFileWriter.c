#include "localFileWriter.h"
#include "util.h"
#include <time.h>

void writeToFile(char* text){
  time_t temp;
  struct tm *timeptr;
  temp = time(NULL);
  timeptr = localtime(&temp);
  char *filename = malloc(15);
  strftime(filename, 14,"%Y%m%d.txt",timeptr);

  FILE *f = fopen(filename, "ab+");

  fprintf(f, "%s", text);

  fclose(f);

  free(filename);
  filename = NULL;
}

void processLocalData(struct subscriber *sub){
  
  char *text = createText(&sub->queue);

  writeToFile(text);

  free(text);
  text = NULL;
}

char *createText(struct queue *q){
  void *item;
  char *reads = malloc(1);
  reads[0] = 0;

  char *stime = timeNow();

  while((item=q->dequeue(q))){
    char* packet = bytesToStringHex(26, (unsigned char*)item);
    free(item);
    item = NULL;
    char *pTemp = malloc(strlen(packet)+30);
    sprintf(pTemp,"%s => %s\n",stime, packet);
    free(packet);
    packet = NULL;
    char *reads1 = realloc(reads, strlen(reads)+strlen(pTemp)+1);
    if(reads1==NULL){
      fprintf(stderr, "No more memory available!");
      exit(200);
    }else{
      reads = reads1;
      reads1 = NULL;
    }
    strcat(reads, pTemp);
    free(pTemp);
    pTemp = NULL;
  }
  free(stime);
  stime = NULL;
  return reads;
}
