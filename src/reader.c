#include <stdio.h>
#include "reader.h"
#include "debug.h"

bool validatePacket(unsigned char *message, int *oz, int count){
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
        debug_print("%02x ",message[g]);
      }
      debug_print("%s\n","");
      sum+=0xaa;
      return sum==message[25];
}
