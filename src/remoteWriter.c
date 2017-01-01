//this file contains the implementation for the REST client

#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "remoteWriter.h"
#include "queue.h"
#include "util.h"

struct curl_slist *headers = NULL;

//BufferStruct will hold the output from curl
struct BufferStruct{
  char *buffer;
  size_t size;
};

CURL *curl;

char *remoteServer;
char *myclientid;

bool RWI;

char *createJson(struct queue *q){
  void *item = NULL;
  char *reads = malloc(1);
  reads[0] = 0;
  while((item=q->dequeue(q))){
    char* packet = bytesToStringHex(26, (unsigned char*)item);
    free(item);
    item = NULL;
    char *pTemp = malloc(strlen(packet)+15);
    sprintf(pTemp,"{\"packet\":\"%s\"}",packet);
    free(packet);
    packet = NULL;
    char *reads1 = NULL;
    if(reads==NULL){
      reads1 = realloc(reads, strlen(pTemp)+20);
    }else{
      reads1 = realloc(reads, strlen(reads)+strlen(pTemp)+20);
    }

    if(reads1==NULL){
      fprintf(stderr, "No more memory available!");
      exit(200);
    }else{
      reads = reads1;
      reads1 = NULL;
    }
    if(reads){
      if(strlen(reads)!=0){
        strcat(reads,", ");
      }
      strcat(reads, pTemp);
    }
    free(pTemp);
    pTemp = NULL;
  }

  char *stime = timeNow();
  char *sendstring = malloc(strlen(reads)+strlen(myclientid)+strlen(stime)+100);
  sprintf(sendstring,"{\"id\":\"%s\",\"reads\":[%s],\"time\":\"%s\"}",myclientid,reads,stime);
  free(reads);
  reads = NULL;
  free(stime);
  stime = NULL;
  return sendstring;
}

//WriteMemoryCallback will capture the curl output
static size_t WriteMemoryCallback (void *ptr, size_t size,
                size_t nmemb, void *data)
{

  size_t realsize = size * nmemb;
  struct BufferStruct *mem = (struct BufferStruct*) data;
  mem->buffer = realloc(mem->buffer, mem->size +realsize + 1);

  if(mem->buffer)
  {
    memcpy( &(mem->buffer[ mem->size ]), ptr, realsize);
    mem->size += realsize;
    mem->buffer[mem->size] = 0;
  }

  return realsize;

}

//processData will be used by the subscriber to send data
void processData(struct subscriber *sub){
  if(!RWI){
    fprintf(stderr, "Remote writer not inited\n" );
    return;
  }

  CURLcode result;

  struct BufferStruct output;
  output.buffer = NULL;
  output.size = 0;
  long http_code = 0;
  double c_length = 0;
  char errbuf[CURL_ERROR_SIZE];
  errbuf[0]=0;

  curl_global_cleanup();

  startCurl();

  if(!curl){
    fprintf(stderr, "Exiting as curl was not inited\n" );
  }

  char *json = createJson(&sub->queue);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,(void *)&output);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

  result = curl_easy_perform( curl );

  if(result!=CURLE_OK){
    fprintf(stderr, "Could not perform curl request\n");
  }

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &c_length);

  curl_slist_free_all(headers);
  headers = NULL;

  curl_easy_cleanup(curl);

  free(json);
  json = NULL;

  if(http_code!=200L){
    fprintf(stderr, "got CODE: %ld, %f, %s\n", http_code, c_length, errbuf);
  }

  fprintf(stdout, "%s\n", output.buffer);

  if(output.buffer){
    free(output.buffer);
    output.buffer = NULL;
    output.size = 0;
  }

}

void startCurl(void){

  static const char *pCertFile = "./certs/cert.pem";
  static const char *pCACertFile = "./certs/ca-chain.cert.pem";
  static const char *pKeyFile = "./certs/key.pem";

  headers = curl_slist_append(headers, "Content-Type: application-json");

  curl = curl_easy_init();
  if(!curl){
    fprintf(stderr, "Remote writer could not be inited!");
    return;
  }

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  //curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
  curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
  curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);
  curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
  curl_easy_setopt(curl, CURLOPT_SSLKEY, pKeyFile);
  curl_easy_setopt(curl, CURLOPT_CAINFO, pCACertFile);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_URL, remoteServer);

}

void initRemoteWriter(const char* srv,const char* cid){

  remoteServer = strdup(srv);
  myclientid = strdup(cid);

  ///curl_global_init(CURL_GLOBAL_ALL);

  RWI = true;
}

void destroyRemoteWriter(void){
  fprintf(stderr, "Destroying remote writer\n" );
  free(remoteServer);
  free(myclientid);
  remoteServer = NULL;
  myclientid = NULL;
}
