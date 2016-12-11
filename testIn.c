//http://elinux.org/RPi_GPIO_Code_Samples#C
#define BCM2708_PERI_BASE 	0x20000000 //use this for Rasberry 1
//#define BCM2708_PERI_BASE 	0x3F000000 //use this for Raspbery 2 and 3
#define GPIO_BASE 		(BCM2708_PERI_BASE + 0x200000)

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

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

void printButton(int g)
{
  if (GET_GPIO(g)){
    printf("Button pressed!\n");
  }else{
    printf("Button released!\n");
  }
}

int main(int argc, char **argv)
{
  int g, rep;
  struct timespec sBefore;
  struct timespec sAfter;
  long tBefore;
  long tAfter;

  setup_io();
  INP_GPIO(17);
  INP_GPIO(27);

  clock_gettime(CLOCK_MONOTONIC, &sBefore);

  tBefore = sBefore.tv_nsec;

  for(rep=0; rep<10;rep++){
    printf("%d,%d\n",GET_GPIO(17),GET_GPIO(27));
    sleep(1);
  }

  clock_gettime(CLOCK_MONOTONIC, &sAfter);

  tAfter = sAfter.tv_nsec;

  printf("%d, %d, %d\n", tBefore/1000, tAfter/1000, (tAfter-tBefore)/1000);  

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
