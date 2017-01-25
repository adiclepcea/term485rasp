#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "debug.h"
#include "reader.h"
#include "observer.h"
#include <pthread.h>
#include "remoteWriter.h"
#include "localFileWriter.h"
#include "configJson.h"
#include "message.h"
#include "util.h"

#ifdef __arm__
#include "reader485.h"
#else
#include "readerMock.h"
#endif

char *server;
char *clientid;

unsigned char lastCC[26];
unsigned char lastDD[26];

/////////////////////////////////////////////////////////
//this will be used to properly destroy the remoteWriter
//during the "end" action for the subscriber
void (*endRemoteWriter)(struct subscriber *sub);
void newEndRemoteWriter(struct subscriber *sub){
  endRemoteWriter(sub);
  destroyRemoteWriter();
}
//////////////////////////////////////////////////////////

bool readConfigJson(){
  fprintf(stderr, "%s\n", "reading" );
  readConfig("config.json",&clientid,&server);
  if(server==NULL){
    fprintf(stderr, "Configuration value server not found!");
    return false;
  }
  if(clientid==NULL){
    free(server);
    server = NULL;
    fprintf(stderr, "Configuration value clientid not found!");
    return false;
  }
  fprintf(stderr, "got %s,%s\n", clientid,server );
  return true;
}
/*
bool readConfig(){
  server = readConfigValue("config.ini", "server");
  clientid = readConfigValue("config.ini", "clientid");
  heartbeatserver = readConfigValue("config.ini","heartbeatserver");

  if(server==NULL){
    fprintf(stderr, "Configuration value server not found!");
    return false;
  }
  if(clientid==NULL){
    free(server);
    server = NULL;
    fprintf(stderr, "Configuration value clientid not found!");
    return false;
  }
  if(heartbeatserver==NULL){
    free(server);
    server = NULL;
    free(clientid);
    clientid = NULL;
    fprintf(stderr, "Configuration value for heartbeatserver not found!");
  }

  return true;
}
*/
void clean(){
  if(clientid!=NULL){
    free(clientid);
    clientid = NULL;
  }
  if(server){
    free(server);
    server = NULL;
  }

}

static inline bool theSameAsLast(unsigned char *message){
  int i;
  int j;
  unsigned char *last = NULL;
  if(message[0]==0xCC){
    last = lastCC;
  }else if(message[0]==0xDD){
    last = lastDD;
  }
  if(last==NULL){ //we return true although this is not so, but the package should never be sent;
    return true;
  }
  for(i=0;i<26;i++){
    if(message[i]!=last[i]){
      for(j=0;j<26;j++){//we copy the new message in place of the old one;
        last[j]=message[j];
      }
      last = NULL;
      return false;
    }
  }
  return true;
}

int main(int argc, char **argv)
{

  int poz[208];
  long period[208];
  int oz[208];

  clientid = NULL;
  server = NULL;

  if(!readConfigJson()){
    clean();
    exit(-1);
  }

//choose the read source
#ifdef __arm__
  Reader reader485;
  reader485.readPacket = readPacket485;
  reader485.prepare = prepare485;
  reader485.validatePacket = validatePacket;

  Reader reader = reader485;
#else
  Reader readerTest;
  readerTest.readPacket = readPacketMock;
  readerTest.prepare = prepareMock;
  readerTest.validatePacket = validatePacket;

  Reader reader = readerTest;
#endif

//set up the publisher for the obeserver pattern
  Publisher pub;
  pub.init = initPublisher;
  pub.init(&pub);

//add the needed subscribers
  initRemoteWriter(server,clientid);

  //initRemoteWriter("https://192.168.1.247:8080/hello","me");

  Subscriber sRemoteWriter;
  sRemoteWriter.init = initSubscriber;
  sRemoteWriter.init(&sRemoteWriter,processData);
  //printf("%d\n",sRemoteWriter.queue.noOfItems);
  ///////
  //we add the remoteWriter destroy action to the subscriber;
  //endRemoteWriter = sRemoteWriter.end;
  //sRemoteWriter.end = newEndRemoteWriter;
  ///////

  Subscriber sLocalFileWriter;
  sLocalFileWriter.init = initSubscriber;
  sLocalFileWriter.init(&sLocalFileWriter,processLocalData);

  pub.subscribe(&pub, &sRemoteWriter);
  pub.subscribe(&pub, &sLocalFileWriter);

//start reading
  reader.prepare();

  int reads = 0;

  ReadMessage rm;

  while(reads<40){

    int count = reader.readPacket(poz,period, oz);

    if(count<208){
      printf("Incorrect package: length is only %d \n", count);
    }
    else{
     //unsigned char message[26];

     if(reader.validatePacket(rm.message, oz,count)){
       if(!theSameAsLast(rm.message)){
         char *t = timeNow();
         strncpy(rm.time_str, t,20);
         free(t);
         t = NULL;
         pub.publish(&pub, sizeof(rm),&rm);
       }
     }

    }
    reads++;
    usleep(5000);
  }

  sleep(2);

  clean();

  pub.unsubscribe(&pub, &sRemoteWriter);
  pub.unsubscribe(&pub, &sLocalFileWriter);
  pthread_join(sRemoteWriter.thread,NULL);
  pthread_join(sLocalFileWriter.thread,NULL);
  pthread_exit(NULL);

  return 0;

}
