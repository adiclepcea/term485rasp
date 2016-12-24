#include <stdio.h>
#include <unistd.h>

#include "reader.h"
#ifdef __arm__
#include "reader485.h"
#else
#include "readerMock.h"
#endif

static inline bool validatePacket(unsigned char *message, int *oz, int count){
  int rep,g;
   for(g=0;g<count;g+=8){
        int val = 0x00;
        for(rep=0;rep<8;rep++){
          val = val | (oz[g+rep] << rep);
        }
        message[g/8]=val;
      }
      unsigned char sum = 0;
      for(g=0;g<26;g++){
        if(g!=25){
          sum+=message[g];
        }
        printf("%02x ",message[g]);
      }
      sum+=0xaa;
      printf("\n");

}

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
