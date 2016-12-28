#include <stdio.h>
#include <unistd.h>
#include "debug.h"
#include "reader.h"
#include "observer.h"
#include <pthread.h>
#include "remoteWriter.h"

#ifdef __arm__
#include "reader485.h"
#else
#include "readerMock.h"
#endif

/////////////////////////////////////////////////////////
//this will be used to properly destroy the remoteWriter
//during the "end" action for the subscriber
void (*endRemoteWriter)(struct subscriber *sub);
void newEndRemoteWriter(struct subscriber *sub){
  endRemoteWriter(sub);
  destroyRemoteWriter();
}
/////////////////////////////////////////////////////////

int main(int argc, char **argv)
{

  int poz[208];
  long period[208];
  int oz[208];

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
  initRemoteWriter("https://127.0.0.1:8080/hello","myclientid");
  Subscriber sRemoteWriter;
  sRemoteWriter.init = initSubscriber;
  sRemoteWriter.init(&sRemoteWriter,processData);
  ///////
  //we add the remoteWriter destroy action to the subscriber;
  endRemoteWriter = sRemoteWriter.end;
  sRemoteWriter.end = newEndRemoteWriter;
  ///////
  pub.subscribe(&pub, &sRemoteWriter);

//start reading
  reader.prepare();

  int reads = 0;

  while(reads<1000){

    int count = reader.readPacket(poz,period, oz);

    if(count<208){
      printf("Incorrect package: length is only %d \n", count);
    }
    else{
     unsigned char message[26];

     if(reader.validatePacket(message, oz,count)){
       pub.publish(&pub, sizeof(message),message);
     }

    }
    reads++;
    usleep(5000);
  }

  pub.unsubscribe(&pub, &sRemoteWriter);

  pthread_exit(NULL);

  return 0;

}
