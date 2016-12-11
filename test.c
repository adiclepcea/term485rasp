#define BCM2708_PERI_BASE 	0x3F000000
#define GPIO_BASE 		(BCM2708_PERI_BASE + 0x200000)

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

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

  setup_io();

  for(g=7; g<=11; g++){
    INP_GPIO(g);
    OUT_GPIO(g);
  }

  for(rep=0; rep<10; rep++){
    for(g=7; g<11; g++){
      GPIO_SET = 1<<g;
      sleep(1);
    }
    for(g=7;g<11;g++){
      GPIO_CLR = 1<<g;
      sleep(1);
    }
  }

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
    printf("mmap error %d\n", (int)gpio_map);
    exit(-1);
  }

  gpio = (volatile unsigned *)gpio_map;
}
