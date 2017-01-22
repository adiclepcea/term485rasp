#include "jsmn.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <configJson.h>

char* readFile(FILE *file){
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *filecontents = malloc(fsize + 1);
  fread(filecontents,fsize,1, file);
  fclose(file);
  filecontents[fsize] = '\0';

  return filecontents;
}

char* getKeyValue(jsmntok_t *tokens,const char *json,char* keyValue, int startToken, int maxTokens){
  if(startToken>=maxTokens){
    return NULL;
  }
  char *key = malloc(tokens[startToken].end-tokens[startToken].start+1);
  char *value = malloc(tokens[startToken+1].end-tokens[startToken+1].start+1);
  strncpy(key,json+tokens[startToken].start, tokens[startToken].end-tokens[startToken].start);
  key[tokens[startToken].end-tokens[startToken].start] = '\0';
  strncpy(value,json+tokens[startToken+1].start, tokens[startToken+1].end-tokens[startToken+1].start);
  value[tokens[startToken+1].end-tokens[startToken+1].start] = '\0';

  int keyType = tokens[startToken].type;
  int valueType = tokens[startToken+1].type;

  if(keyType!=JSMN_STRING){
    fprintf(stderr, "%s\n","Wrong key type");
  }else{
    if(strcmp(keyValue, key)==0){
      free(key);
      key = NULL;
      return value;
    }else{
      if(valueType==JSMN_STRING){
        free(key);
        key = NULL;
        free(value);
        value = NULL;
        return getKeyValue(tokens, json,keyValue,startToken+2,maxTokens);
      }else{ //we do not support reading pass arrays
        return NULL;
      }
    }
  }
  return NULL;
}

void readParams(char *filecontents,char **clientid, char **reader_server, char *json){
  jsmn_parser parser;
  jsmn_init(&parser);

  jsmntok_t tokens[256];

  int r = jsmn_parse(&parser, filecontents, strlen(filecontents),tokens, 256);

  if(r<0){
    switch(r){
      case JSMN_ERROR_INVAL:
        fprintf(stderr, "%s","Incorrect JSON\n");
        break;
      case JSMN_ERROR_NOMEM:
        fprintf(stderr, "%s", "JSON is too large\n");
        break;
      case JSMN_ERROR_PART:
        fprintf(stderr, "%s", "JSON is too short. Data missing\n");
        break;
      default:
        fprintf(stderr, "%s", "Unknown error\n");
    }
    return;
  }else{
    *reader_server=getKeyValue(tokens, json, "reader_server",1,r);
    *clientid=getKeyValue(tokens, json, "clientid",1,r);
  }

  return;
}

void readConfig(const char *config,char **clientid,char **reader_server){
  char *filecontents = NULL;

  if(access(config, F_OK) != -1){
      FILE *file = fopen(config,"rb");
      filecontents = readFile(file);
  }else{
    fprintf(stderr, "Config file not found");
    exit(-1);
  }

  readParams(filecontents, clientid, reader_server, filecontents);

  if(filecontents!=NULL){
    free(filecontents);
    filecontents = NULL;
  }

}
