//this file contains the implementation for the REST client

#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "remoteWriter.h"

//BufferStruct will hold the output from curl
struct BufferStruct{
  char *buffer;
  size_t size;
};

CURL *curl;

char *server;
char *clientid;

bool RWI;

char *bytesToStringHex(size_t size, unsigned char *array){
  int i;
  char *out = malloc(size*2+1);
  out[0]=0;
  char tmp[3];
  for(i=0;i<size;i++){
    sprintf(tmp,"%02x",array[i]);
    strcat(out, tmp);
  }
  out[size*2+1]=0;
  return out;
}

char *createJson(size_t size, unsigned char *array){
  char* packet = bytesToStringHex(size, array);
  time_t temp;
  struct tm *timeptr;
  temp = time(NULL);
  timeptr = localtime(&temp);
  char stime [100];
  strftime(stime, sizeof(stime),"%Y-%m-%dT%H:%M:%S",timeptr);
  char *sendstring = malloc(strlen(packet)+strlen(clientid)+strlen(stime)+30);
  sprintf(sendstring,"{\"id\":\"%s\",\"packet\":\"%s\",\"time\":\"%s\"}",clientid,packet,stime);
  free(packet);
  packet = NULL;
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
  long http_code;
  double c_length;
  char errbuf[CURL_ERROR_SIZE];
  errbuf[0]=0;


  startCurl();

  if(!curl){
    fprintf(stderr, "Exiting as curl was not inited\n" );
  }

  char *json = createJson(sub->dataSize, sub->data);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,(void *)&output);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

  result = curl_easy_perform( curl );

  if(result!=CURLE_OK){
    fprintf(stderr, "Could not perform curl request\n");
  }

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &c_length);

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
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "charsets: utf-8");

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();
  if(!curl){
    fprintf(stderr, "Remote writer could not be inited!");
    return;
  }

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
  curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
  curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);
  curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
  curl_easy_setopt(curl, CURLOPT_SSLKEY, pKeyFile);
  curl_easy_setopt(curl, CURLOPT_CAINFO, pCACertFile);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, server);

}

void initRemoteWriter(const char* srv,const char* cid){

  server = malloc(strlen(srv)+1);
  clientid = malloc(strlen(cid)+1);
  strcpy(server,srv);
  strcpy(clientid,cid);

  RWI = true;
}

void destroyRemoteWriter(void){
  fprintf(stderr, "Destroying remote writer\n" );
  free(server);
  free(clientid);
  server = NULL;
  clientid = NULL;
}
