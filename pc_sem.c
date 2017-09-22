#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

struct buff{
   uthread_sem_t mutex;
   uthread_sem_t emptycells;
   uthread_sem_t fullcells;
   uthread_sem_t barrier;
  int           items;

};

struct buff* createBuff(){
 struct buff* buff = malloc(sizeof(struct buff));
 buff-> mutex = uthread_sem_create(0);
 buff-> emptycells = uthread_sem_create(MAX_ITEMS);
 buff-> fullcells = uthread_sem_create(0);
 buff-> barrier = uthread_sem_create(0);
 buff->items = 0;
}

void* producer (void* bv) {
  struct buff* b = bv;
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
  uthread_sem_wait(b->emptycells);
  producer_wait_count++;
  uthread_sem_wait(b->mutex);
  b->items++;
  assert(b->items>=0 && b->items <= MAX_ITEMS);
  histogram[b->items]++;
  uthread_sem_signal(b->fullcells);
  uthread_sem_signal(b->mutex);

  }
  uthread_sem_signal(b->barrier);
  return NULL;
}

void* consumer (void* bv) {
 struct buff* b = bv;
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
  uthread_sem_wait(b->fullcells);
  consumer_wait_count++;
  uthread_sem_wait(b->mutex);
  b->items--;
  assert(b->items>=0 && b->items <= MAX_ITEMS);
  histogram[b->items]++;
  uthread_sem_signal(b->emptycells);
  uthread_sem_signal(b->mutex);

  }
  uthread_sem_signal(b->barrier);
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);
  struct buff* b = createBuff();
for(int i=0; i<2; i++){
  t[i]=uthread_create(producer, b);
}
for(int i=2; i<4; i++){
  t[i]=uthread_create(consumer, b);
}
for(int i=0; i<4; i++){
//uthread_join(t[i],0);
  uthread_sem_wait(b->barrier);
}
  // TODO: Create Threads and Join

  printf ("producer_wait_count=%d, consumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
