#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

#define MAX_OCCUPANCY      3
#define NUM_ITERATIONS     100
#define NUM_PEOPLE         20
#define FAIR_WAITING_COUNT 4

/**
 * You might find these declarations useful.
 */
enum GenderIdentity {MALE = 0, FEMALE = 1};
const static enum GenderIdentity otherGender [] = {FEMALE, MALE};

struct Washroom {
  // TODO
  uthread_mutex_t mutex;
  uthread_cond_t canEnter[2];
  enum  GenderIdentity occupantGender;
  int totalWaiter;
  int occupantNumber;
  int genderNumer[2];
  int otherGenderWaiting;
};

struct Washroom* createWashroom() {
  struct Washroom* washroom = malloc (sizeof (struct Washroom));
  // TODO
  washroom->mutex = uthread_mutex_create();
  washroom->canEnter[MALE] = uthread_cond_create(washroom->mutex);
  washroom->canEnter[FEMALE] = uthread_cond_create(washroom->mutex);
  washroom->occupantGender = MALE;
  washroom->totalWaiter = 0;
  washroom->occupantNumber=0;
  washroom->genderNumer[MALE]=0;
  washroom->genderNumer[FEMALE]=0;
  washroom->otherGenderWaiting=0;
  return washroom;
}

struct Washroom* washroom;

#define WAITING_HISTOGRAM_SIZE (NUM_ITERATIONS * NUM_PEOPLE)
int             entryTicker;                                          // incremented with each entry
int             waitingHistogram         [WAITING_HISTOGRAM_SIZE];
int             waitingHistogramOverflow;
uthread_mutex_t waitingHistogrammutex;
int             occupancyHistogram       [2] [MAX_OCCUPANCY + 1];

void enterWashroom (struct Washroom* w,enum GenderIdentity g) {
  // TODO
 uthread_mutex_lock(w->mutex);
 while(1){
 int isEmpty;
  int hasRoom;
  int sameGender;
 
  int unfair;
  int shouldTurn;
  if(w->occupantNumber==0)
   isEmpty=1;
 else
   isEmpty=0;
 if(w->occupantNumber<MAX_OCCUPANCY)
   hasRoom=1;
 else
   hasRoom=0;
 if(g== w->occupantGender)
   sameGender=1;
 else
   sameGender=0;
 if(w->otherGenderWaiting > FAIR_WAITING_COUNT)
   unfair=1;
 else
   unfair=0;
 if(unfair && w->genderNumer[otherGender[g]]>0)
  shouldTurn=1;
else
  shouldTurn=0;
 if(isEmpty || (hasRoom && sameGender && !shouldTurn)){
  if(hasRoom && sameGender && !shouldTurn)
  w->otherGenderWaiting ++;
 else
  w->otherGenderWaiting = 0;
entryTicker++;
break;
 }
 if(!sameGender && w->genderNumer[g])
  w->otherGenderWaiting=0;
w->genderNumer[g]++;
uthread_cond_wait(w->canEnter[g]);

w->genderNumer[g]--;
}
assert(w->occupantNumber<MAX_OCCUPANCY);
assert(w->occupantGender==g || w->occupantNumber==0);
w->occupantNumber++;
w->occupantGender=g;
occupancyHistogram[w->occupantGender][w->occupantNumber]++;
uthread_mutex_unlock(w->mutex);

}

void leaveWashroom(struct Washroom* w) {
  // TODO
  uthread_mutex_lock(w->mutex);
  int ifoccupanciedGenderWait;
  int ifotherGenderWait;
  int unfair;
  
  if(w->genderNumer[w->occupantGender]>0)
    ifoccupanciedGenderWait=1;
  else
    ifoccupanciedGenderWait=0;
  if(w->genderNumer[otherGender[w->occupantGender]]>0)
    ifotherGenderWait=1;
  else
    ifotherGenderWait=0;
  if(w->otherGenderWaiting > FAIR_WAITING_COUNT)
   unfair=1;
 else
   unfair=0;
 w->occupantNumber--;
 if(ifotherGenderWait && (unfair || !ifoccupanciedGenderWait)){
   if(w->occupantNumber == 0){
    for(int i=0; i< MAX_OCCUPANCY; i++){
    uthread_cond_signal(w->canEnter[otherGender[w->occupantGender]]);
   }
 }
 }else if(ifoccupanciedGenderWait){
  uthread_cond_signal(w->canEnter[w->occupantGender]);

 }
 uthread_mutex_unlock(w->mutex);
}

void recordWaitingTime (int waitingTime) {
  uthread_mutex_lock (waitingHistogrammutex);
  if (waitingTime < WAITING_HISTOGRAM_SIZE)
    waitingHistogram [waitingTime] ++;
  else
    waitingHistogramOverflow ++;
  uthread_mutex_unlock (waitingHistogrammutex);
}

//
// TODO
// You will probably need to create some additional produres etc.
//
void* person(void* washroomv){
struct Washroom* w = washroomv;
enum GenderIdentity g = random()%1;

for(int i=0; i<NUM_ITERATIONS; i++){
  int start = entryTicker;
  enterWashroom(w,g);
  recordWaitingTime(entryTicker-start);
  for(int j=0;j<NUM_PEOPLE ; j++)
    uthread_yield();
  leaveWashroom(w);
  for(int m=0;m<NUM_PEOPLE ; m++)
    uthread_yield();
}
return NULL;
}

void mysrandomdev() {
  unsigned long seed;
  int f = open ("/dev/random", O_RDONLY);
  read    (f, &seed, sizeof (seed));
  close   (f);
  srandom (seed);
}

int main (int argc, char** argv) {
  uthread_init (1);
  mysrandomdev();
  struct Washroom* w = createWashroom();
  uthread_t pt [NUM_PEOPLE];
  waitingHistogrammutex = uthread_mutex_create ();

   for (int i = 0; i < NUM_PEOPLE; i++)
    pt [i] = uthread_create (person, w);
  for (int i = 0; i < NUM_PEOPLE; i++)
    uthread_join (pt [i], 0);

  // TODO
  
  printf ("Times with 1 person who identifies as male   %d\n", occupancyHistogram [MALE]   [1]);
  printf ("Times with 2 people who identifies as male   %d\n", occupancyHistogram [MALE]   [2]);
  printf ("Times with 3 people who identifies as male   %d\n", occupancyHistogram [MALE]   [3]);
  printf ("Times with 1 person who identifies as female %d\n", occupancyHistogram [FEMALE] [1]);
  printf ("Times with 2 people who identifies as female %d\n", occupancyHistogram [FEMALE] [2]);
  printf ("Times with 3 people who identifies as female %d\n", occupancyHistogram [FEMALE] [3]);
  printf ("Waiting Histogram\n");
  for (int i=0; i<WAITING_HISTOGRAM_SIZE; i++)
    if (waitingHistogram [i])
      printf ("  Number of times people waited for %d %s to enter: %d\n", i, i==1?"person":"people", waitingHistogram [i]);
  if (waitingHistogramOverflow)
    printf ("  Number of times people waited more than %d entries: %d\n", WAITING_HISTOGRAM_SIZE, waitingHistogramOverflow);
}
