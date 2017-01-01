#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

char* readConfigValue(char *filename, char* key){
  if(filename==NULL){
    return NULL;
  }
  FILE *f = fopen(filename,"r");

  if(f==NULL){
    fprintf(stderr,"Could not open file for reading\n");
    return NULL;
  }

  char line[300];

  while(fgets(line, 300, f)){
    char *pch = strchr(line, '=');
    char *ctemp = pch;
    ctemp--;

    while(ctemp>line && isspace(*ctemp)){
      *ctemp = 0;
      ctemp--;
    }
    ctemp = NULL;

    pch[0]=0;
    pch+=1;

    if(pch!=NULL){
      while(pch!=NULL && isspace(*pch))
	       pch++;

      ctemp = pch+strlen(pch)-1;
      while(ctemp>pch && isspace(*ctemp)){
        *ctemp = 0;
        ctemp--;
      }
      ctemp = NULL;
      if(strcmp(key,line)==0){
        char *ret = strdup(pch);
        if(ret==NULL){
          fprintf(stderr, "No more memory available\n" );
        }
        pch = NULL;
        fclose(f);
        return ret;
      }
    }
  }

  fclose(f);


  return NULL;
}
