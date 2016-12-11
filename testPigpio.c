#include <stdio.h>
#include <pigpio.h>

int main(int argc, char *argv[]){
  double start;
  
  if(gpioInitialise()<0){
    fprintf(stderr, "pigpio initialization failed");
    return 1;
  }
  gpioSetMode(17, PI_INPUT);

  printf("%f\n", time_time());
  start = time_time();

  while((time_time()-start)<60.0){
    printf("%d\n", gpioRead(17));
    time_sleep(0.5);
  }
  return 0;
}

