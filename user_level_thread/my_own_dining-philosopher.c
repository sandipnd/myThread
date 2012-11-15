#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "mt.h"

#define N	5	/* Must be 5 */
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT (ph_num+4)%N
#define RIGHT (ph_num+1)%N	/* avg eat time in milliseconds */

typedef struct {
  int id;                /* The philosopher's id: 0 to 5 */
  long t0;               /* The time when the program started */
  long ms;               /* The maximum time that philosopher sleeps/eats */
  mt_sem *v;               /* The void * that you define */
  int *blocktime;      /* Total time that a philosopher is blocked */
  int phil_count;
} Phil_struct;

int state[N];
int phil_num[N]={0,1,2,3,4};
mt_sem state_mutex[N];
mt_sem mutex;
mt_sem screen;

void take_fork(int);
void put_fork(int);
void test(int);
void *philosopher(void *);
int main()
{
    int i;
int thread_id[10];
    mt_init();
    mutex=mt_sem_create(1);
    for(i=0;i<N;i++)
        state_mutex[i]=mt_sem_create(0);
    for(i=0;i<N;i++)
    {
        thread_id[i]=mt_create(philosopher,phil_num[i]);
        printf("Philosopher %d is thinking\n",i+1);
    }

  mt_joinall();
    
}
void* philosopher(void *num) 
{
  int x=(int *)num;
  
   while(1)
    {
     mt_sleep(1);  
     take_fork(x);
     mt_sleep(1);
     put_fork(x);
     }

}
void take_fork(int ph_num)
{
    mt_sem_down(mutex);
    state[ph_num] = HUNGRY;
  
    printf("Philosopher %d is Hungry\n",ph_num+1);
    test(ph_num);
    mt_sem_up(mutex);
    mt_sem_down(state_mutex[ph_num]);
    mt_sleep(1);
}

void test(int ph_num)
{
    if (state[ph_num] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
    {
        state[ph_num] = EATING;
        
        printf("Philosopher %d takes fork %d and %d\n",ph_num+1,LEFT+1,ph_num+1);
        printf("Philosopher %d is Eating\n",ph_num+1);
        mt_sem_up(state_mutex[ph_num]);
    }
}

 
void put_fork(int ph_num)
{
    mt_sem_down(mutex);
    state[ph_num] = THINKING;
    printf("Philosopher %d putting fork %d and %d down\n",ph_num+1,LEFT+1,ph_num+1);
    printf("Philosopher %d is thinking\n",ph_num+1);
    test(LEFT);
    test(RIGHT);
    mt_sem_up(mutex);
}


