#include "util.h"

char *bytesToStringHex(size_t size, unsigned char *array){
  int i;
  char *out = malloc(size*2+1);
  out[0]=0;
  char tmp[3];
  for(i=0;i<size;i++){
    sprintf(tmp,"%02x",array[i]);
    strcat(out, tmp);
  }
  out[size*2]=0;
  return out;
}

char *timeNow(){
  time_t temp;
  struct tm *timeptr;
  temp = time(NULL);
  timeptr = localtime(&temp);
  char *stime = malloc(25);
  strftime(stime, 24,"%Y-%m-%dT%H:%M:%S",timeptr);
  return stime;
}
