#include <stdio.h>
#include <stdlib.h>
#include "observer.h"
#include "queue.h"
#include <stdbool.h>

#ifndef LOCAL_FILE_WRITER
  #define LOCAL_FILE_WRITER
FILE *localFile;
bool LFW;

void processLocalData(struct subscriber *sub);
void writeToFile(char *text);
char *createText(struct queue *q);

#endif
