#include <stdbool.h>

#ifndef READER

typedef struct{
   int (*readPacket)(int *poz, long *period, int *oz);
   bool (*prepare)(void);
   bool (*validatePacket)(unsigned char *message, int *oz, int count);
} Reader;

#define READER

#endif

bool validatePacket(unsigned char *message, int *oz, int count);

#define DEBUG 1
