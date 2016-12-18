//http://elinux.org/RPi_GPIO_Code_Samples#C
#define BCM2708_PERI_BASE 	0x20000000 //use this for Rasberry 1
//#define BCM2708_PERI_BASE 	0x3F000000 //use this for Raspbery 2 and 3
#define GPIO_BASE 		(BCM2708_PERI_BASE + 0x200000)

#define RO_PORT 17

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define PAGE_SIZE	(4*1024)
#define BLOCK_SIZE	(4*1024)

int mem_fd;
void *gpio_map;
volatile unsigned *gpio;

#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)

#define GET_GPIO(g) (*(gpio+13)&(1<<g))

#define GPIO_PULL *(gpio+37) 
#define GPIO_PULLCLK0 *(gpio+38)

void setup_io();

int readPacket(int *poz, long *period, int *oz){

  int g, rep; 
  struct timespec sBefore;
  struct timespec sAfter;
  long tBefore;
  long tAfter;
  unsigned long periodBetween;

  int count = 0;
  bool state = false;
  bool packetStart0;
  bool packetStart1;
  while(clock_gettime(CLOCK_MONOTONIC, &sBefore)==-1);

  tBefore = sBefore.tv_nsec;

  while(count<208){
    if(GET_GPIO(RO_PORT)){
      if(!state){
	state = true;
	while(clock_gettime(CLOCK_MONOTONIC, &sAfter)==-1) {printf("Wrong clock 0\n");usleep(1);};
        tAfter = sAfter.tv_nsec;
        
        periodBetween = ((unsigned long)tAfter-(unsigned long)tBefore)/(unsigned long)10000;
        
        if(periodBetween>400 && periodBetween<600){ //1 pos took 5 ms
          packetStart0 = true;
        }

        while(clock_gettime(CLOCK_MONOTONIC, &sBefore)==-1) {printf("Wrong clock 1\n");usleep(1);};
      
        tBefore = sBefore.tv_nsec;
      }
    }else{
      if(state){
        state = false;
	while(clock_gettime(CLOCK_MONOTONIC, &sAfter)==-1){printf("Wrong clock 2\n");usleep(1);};
        tAfter = sAfter.tv_nsec;

        periodBetween = ((unsigned long)tAfter-(unsigned long)tBefore)/(unsigned long)10000;

        if(periodBetween>400 && periodBetween<600){
	  if(packetStart0) packetStart1 = true;       
        }else if(packetStart1){
          poz[count] = 1;
          period[count] = periodBetween;
          oz[count++] = (periodBetween>200) ? 1 : (periodBetween<150 && periodBetween>40 ? 0: 2222);
        }
        while(clock_gettime(CLOCK_MONOTONIC, &sBefore)==-1) {printf("Wrong clock 3\n");usleep(1);};
        tBefore = sBefore.tv_nsec;
      }
    }
    usleep(2);
  }
  return count;
}

int main(int argc, char **argv)
{

  int g, rep;
  int poz[210];
  long period[210];
  int oz[210];

  setup_io();
  INP_GPIO(RO_PORT);

  int reads = 0;

  while(reads<100){
    for(g=0;g<210;g++){
      poz[g]=0;
      period[g]=0;
      oz[g]=0;
    }

    int count = readPacket(poz,period, oz);
  
    if(count<208){
      printf("Incorrect package: length is only %d \n", count);
    }
    else{
      unsigned char message[26];

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
        printf("%02x, ",message[g]);
        //for(rep=0;rep<8;rep++){
        //  printf("%d,",oz[g*8+rep]);
        //}
        //printf("__\t");
      }
      sum+=0xaa;
      printf("crc=%02x\n",sum);
    }
    reads++;
    usleep(5000);
  }
  
  //for(g=0;g<count;g++){
  //  printf("%d - %d - %d\n", poz[g], period[g], oz[g]);
  //}
  return 0;

}

void setup_io()
{
  if((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) <0) {
    printf("Can't open /dev/mem");
    exit(-1);
  }

  gpio_map = mmap(
    NULL,
    BLOCK_SIZE,
    PROT_READ|PROT_WRITE,
    MAP_SHARED,
    mem_fd,
    GPIO_BASE);

  close(mem_fd);

  if(gpio_map == MAP_FAILED) {
    printf("mmap error %ul\n", (int)gpio_map);
    exit(-1);
  }

  gpio = (volatile unsigned *)gpio_map;
}
