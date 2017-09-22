#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include "spinlock.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items


struct  Cond
{
  uthread_mutex_t  mutex;
 uthread_cond_t conp;
 uthread_cond_t conc;
 int items ;
};
struct  Cond* createCond()
{
  struct Cond* c = malloc (sizeof (struct Cond));
  c->mutex = uthread_mutex_create();
  c->conp=uthread_cond_create (c->mutex);
  c->conc=uthread_cond_create (c->mutex);
  c->items = 0;
  return c;
}
void* producer (void* v) {
  struct Cond* c =v;
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    uthread_mutex_lock(c->mutex);
      while(c->items>=MAX_ITEMS){     
        producer_wait_count++;
        uthread_cond_wait (c->conp);
      }
    assert(c->items<MAX_ITEMS);
  c->items++;
  histogram[c->items]++;
  uthread_cond_signal (c->conc);
  uthread_mutex_unlock  (c->mutex);
}
    return NULL;
}

void* consumer (void* v) {
  struct Cond* c =v;
  for (int i=0; i<NUM_ITERATIONS; i++) {
   uthread_mutex_lock(c->mutex);
    while(c->items<=0){
      consumer_wait_count++;
      uthread_cond_wait (c->conc);
    }
    assert(c->items>0);
    c->items--;
    histogram[c->items]++;
    uthread_cond_signal (c->conp);
    uthread_mutex_unlock  (c->mutex);
   }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);
 struct Cond* c = createCond();
  // TODO: Create Threads and Join
  for(int i=0;i<2;i++){
    t[i] = uthread_create  (producer, c);
  }
  for (int i = 2; i < 4; i++)  {
    t[i] = uthread_create (consumer,c);
  }
  for (int i=0; i<4; i++){
    uthread_join (t[i], 0);
  }
  printf ("producer_wait_count=%d, consumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}

/*
int main (int argc, char** argv) {
    uthread_init (16);
  spinlock_create(&lock);
   uthread_t produce[NUM_PRODUCERS];
  uthread_t consume[NUM_CONSUMERS];
  // TODO: Create Threads and Join
  for(int i=0;i<NUM_PRODUCERS;i++){
    produce[i] = uthread_create  (producer, 0);
  }
  for (int i = 0; i < NUM_CONSUMERS; i++)  {
    consume[i] = uthread_create (consumer,0);
  }
  for (int i=0; i<NUM_PRODUCERS; i++){
    uthread_join (produce[i], 0);
  }
  for (int i=0; i<NUM_CONSUMERS; i++){
    uthread_join (consume[i], 0);
  }

  printf ("producer_wait_count=%d, consumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
 // assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
*/