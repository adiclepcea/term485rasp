#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "readerMock.h"

#define MAX_ITEMS 26
#define MAX_LINES 100

unsigned char msg[MAX_LINES][MAX_ITEMS];
int currentLine;

int readPacketMock(int *poz, long *period, int *oz){
  int i,j;
  int pos = 0;
  if(currentLine>=MAX_LINES){ //we loop through the read lines (roundrobin)
    currentLine = 0;
  }
  for(i=0;i<MAX_ITEMS;i++){
    unsigned char item = 0;
    item = msg[currentLine][i];
    for(j=0;j<8;j++){
      oz[pos] = (item >> j) & 1;
      pos++;
    }
  }
  currentLine++;
  usleep(500000);//we sleep half of seccond to simulate the real communication
  return pos;
}

bool prepareMock(){
  char line[100];
  int count = 0;
  FILE *f = fopen("./assets/output.txt", "r");

  while(fgets(line,100,f) && count<MAX_LINES){
    char *pch;
    unsigned char msgPart;
    int linePos=0;
    pch = strtok(line," ");
    while(pch!=NULL && linePos<MAX_ITEMS){
      sscanf(pch,"%hhx",&msgPart);
      msg[count][linePos++]=msgPart;
      pch = strtok(NULL," ");
    }
    count++;
  }
  fclose(f);

  return true;
}
