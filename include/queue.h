#include <pthread.h>

#ifndef QUEUE
  #define QUEUE
typedef struct queueItem{
  void* item;
  struct queueItem *next;
} QueueItem;

typedef struct queue{
  void (*enqueue)(struct queue *me, void *item);
  void (*destroy)(struct queue *me);
  void* (*dequeue)(struct queue *me);
  struct queueItem *head;
  struct queueItem *tail;
  unsigned int noOfItems;
  pthread_mutex_t mutex;
  unsigned char maxItems;
} Queue;

void initQueue(struct queue *me);

#endif
