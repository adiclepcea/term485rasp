#include <stdbool.h>
#include "reader485.h"

#define TIMEPERIOD(AFTER,BEFORE) ((AFTER.tv_sec-BEFORE.tv_sec)*1000000000+(after.tv_nsec - before.tv_nsec))/10000;

static inline unsigned long long  timeperiod(struct timespec after, struct timespec before){
  return ((after.tv_sec-before.tv_sec)*1000000000 + (after.tv_nsec - before.tv_nsec))/10000;
}

int readPacket485(int *poz, long *period, int *oz){

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

        periodBetween = timeperiod(sAfter,sBefore);

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

        periodBetween = timeperiod(sAfter, sBefore);

        if(periodBetween>400 && periodBetween<600){
	        if(packetStart0) packetStart1 = true;
        }else if(packetStart1){
          poz[count] = 1;
          period[count] = periodBetween;
          oz[count++] = (periodBetween>200 && periodBetween<300) ? 1 : (periodBetween<150 && periodBetween>40 ? 0: 2222);
        }
        while(clock_gettime(CLOCK_MONOTONIC, &sBefore)==-1) {printf("Wrong clock 3\n");usleep(1);};
      }
    }
    usleep(2);
  }
  return count;
}

bool prepare485(){
  setup_io();
  INP_GPIO(RO_PORT);
  return true;
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
    printf("mmap error %ul\n", gpio_map);
    exit(-1);
  }

  gpio = (volatile unsigned *)gpio_map;
}
