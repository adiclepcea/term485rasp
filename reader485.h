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

int readPacket485(int *poz, long *period, int *oz);

bool prepare485();

void setup_io();
