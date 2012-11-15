/*
 * dphil_skeleton.c -- Dining philosophers driver program 
 */


#include <stdio.h>
#include "mt.h"
#define N	5	/* Must be 5 */
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT (ph_num+4)%N
#define RIGHT (ph_num+1)%N	/* avg eat time in milliseconds */

int DIFF_STATES[N];
int phil_num[N]={0,1,2,3,4};
mt_sem state_mutex[N];
mt_sem mutex;


typedef struct {
  int id;                /* The philosopher's id: 0 to 5 */
  long t0;               /* The time when the program started */
  long ms;               /* The maximum time that philosopher sleeps/eats */
  void *v;               /* The void * that you define */
  int *blocktime;      /* Total time that a philosopher is blocked */
  int phil_count;
} Phil_struct;


#define MAXTHREADS (10)

void pickup(Phil_struct *philosopher)
{   
    int ph_num = philosopher->id;
    mt_sem_down(mutex);
    DIFF_STATES[ph_num] = HUNGRY;
  
    printf("Philosopher %d is Hungry\n",ph_num+1);
    test(ph_num);
    mt_sem_up(mutex);
    mt_sem_down(state_mutex[ph_num]);
    mt_sleep(1);
}

void test(int ph_num)
{
    if (DIFF_STATES[ph_num] == HUNGRY && DIFF_STATES[LEFT] != EATING && DIFF_STATES[RIGHT] != EATING)
    {
        DIFF_STATES[ph_num] = EATING;
        
        printf("Philosopher %d takes fork %d and %d\n",ph_num+1,LEFT+1,ph_num+1);
        printf("Philosopher %d is Eating\n",ph_num+1);
        mt_sem_up(state_mutex[ph_num]);
    }
}

 
void putdown(Phil_struct *philosopher)
{
    int ph_num = philosopher->id;
    mt_sem_down(mutex);
    DIFF_STATES[ph_num] = THINKING;
    printf("Philosopher %d putting fork %d and %d down\n",ph_num+1,LEFT+1,ph_num+1);
    printf("Philosopher %d is thinking\n",ph_num+1);
    test(LEFT);
    test(RIGHT);
    mt_sem_up(mutex);
}


void philosopher(void *v)
{
  Phil_struct *ps;
  long st;
  long t;

  ps = (Phil_struct *) v;
  while(1) {

    /* First the philosopher thinks for a random number of seconds */

    st = (random()%ps->ms) + 1;
    printf("%3d Philosopher %d thinking for %d second%s\n",
                time(0)-ps->t0, ps->id, st, (st == 1) ? "" : "s");
    fflush(stdout);
    mt_sleep(st);

    /* Now, the philosopher wakes up and wants to eat.  He calls pickup
       to pick up the chopsticks */

    printf("%3d Philosopher %d no longer thinking -- calling pickup()\n", 
            time(0)-ps->t0, ps->id);
    fflush(stdout);
    t = time(0);
    pickup(ps);
    ps->blocktime[ps->id] += (time(0) - t);

    /* When pickup returns, the philosopher can eat for a random number of
       seconds */

    st = (random()%ps->ms) + 1;
    printf("%3d Philosopher %d eating for %d second%s\n",
                time(0)-ps->t0, ps->id, st, (st == 1) ? "" : "s");

    fflush(stdout);
    mt_sleep(st);

    /* Finally, the philosopher is done eating, and calls putdown to
       put down the chopsticks */

    printf("%3d Philosopher %d no longer eating -- calling putdown()\n", 
            time(0)-ps->t0, ps->id);
    fflush(stdout);
    putdown(ps);
  }
}

main(argc, argv)
int argc; 
char **argv;
{
  int i;
  void *threads[MAXTHREADS];
  Phil_struct ps[MAXTHREADS];
  void *v;
  long t0;
  int *blocktime;
  char s[500];
  int phil_count;
  char *curr;
  int total;

  if (argc != 3) {
    fprintf(stderr, "usage: dphil philosopher_count maxsleepsec\n");
    exit(1);
  }

  srandom(time(0));
  mt_init();
  phil_count = atoi(argv[1]);
  
  if(phil_count > MAXTHREADS)
	  phil_count = MAXTHREADS;
   
  v = (phil_count);
  t0 = time(0);
  blocktime = (int *) malloc(sizeof(int)*phil_count);
  for (i = 0; i < phil_count; i++) blocktime[i] = 0;
  for(i=0;i<N;i++)
        state_mutex[i]=mt_sem_create(0);

  for (i = 0; i < phil_count; i++) {
    ps[i].id = i;
    ps[i].t0 = t0;
    ps[i].v = v;
    ps[i].ms = atoi(argv[2]);
    ps[i].blocktime = blocktime;
    ps[i].phil_count = phil_count;
    threads[i] = mt_create(philosopher, (void *) (ps+i));
     printf("Philosopher %d is thinking\n",i+1);
  }
  
    

  mt_joinall();
 }
