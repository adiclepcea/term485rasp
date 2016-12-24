#include <stdio.h>
#include <unistd.h>
#include "debug.h"
#include "reader.h"

#ifdef __arm__
#include "reader485.h"
#else
#include "readerMock.h"
#endif

int main(int argc, char **argv)
{

  int poz[208];
  long period[208];
  int oz[208];

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
  reader.prepare();

  int reads = 0;

  while(reads<10){

    int count = reader.readPacket(poz,period, oz);

    if(count<208){
      printf("Incorrect package: length is only %d \n", count);
    }
    else{
     unsigned char message[26];
     reader.validatePacket(message, oz,count);
    }
    reads++;
    usleep(5000);
  }

  return 0;

}
