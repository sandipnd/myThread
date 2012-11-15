#include "mt.h"
#include<stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include<unistd.h>
#include <sys/time.h>

#define die(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define MT_DEBUG( string ) fprintf( stderr, "myThread debug: " string "\n")
//define the size of thread , stack size , states of the thread
#define MAX_THREADS 1600
#define STACK_SIZE (1024 * 64)

enum  { FREE , ACTIVE , SLEEP = 3}; // needed when we call sleep
enum { FAILURE , SUCCESS } ;


//define thread structure TCB
typedef struct {
 ucontext_t th_context; /* Stores the current context of the thread */
 void *th_stack;
 short state ;
 short my_thread_id;
 short ISJOIN;
 int time_to_wake_up;
 struct timeval start;
	//struct timeval stop;
} myThread;



static myThread myThreadList[MAX_THREADS]; // all threads in Array

static myThread myThreadWaitList[MAX_THREADS]; // waiting thread queue
// it is 0 for the main thread

static int threadInUse = 0;

// to track whether i am in main or in any thread
static int WHERE_AM_I  = 0;

//volatile int WHERE_AM_I[MAXTHREAD];
static int threadCount = 1 ;


/* The "main" execution context */
static ucontext_t mainContext;

static int KILL = 0;
//Arrange a thread after a job is complete

static int waitCount = 1;

//maintain threadId

static int THREADID = 0;

// Add portion for semaphore #####################################################

 //mutex queue
struct Mtx_Q {
 myThread currThread;
 struct Mtx_Q *next;
};

//semaphore queue
typedef struct  {
 int count;
 struct Mtx_Q *waitQ;
 } semop;
 
#define MUTEX ((struct mt_sem) { 1, NULL })

mt_sem 
 mt_sem_create(int initval) 
 {
   semop *newSem ;
 if ( initval < 0 )
  {
    printf(" the assigned value can't be -ve \n");
    return NULL;
  }
   if ( ( newSem = (semop *) malloc(STACK_SIZE)) == NULL )
    {
       perror(" error while allocating malloc");
       return NULL;
    }
 
   newSem -> count = initval;
   newSem -> waitQ = NULL;
   
 return newSem;
 }

int 
 mt_sem_getval ( mt_sem S )
 {
   semop *typeC = ( semop *)S; 
   if ( typeC == NULL )
     return -1;
   return typeC->count;
 }

//The V operation is to increment the value of the semaphore by 1 and the P operation is to decrement the value of the semaphore by 1 as soon as the resulting value would be non-negative. The two operations are indivisible or atomic
void
  mt_sem_up ( mt_sem ksem )
   {
 
    semop *typeC = (semop *)ksem;
     if ( typeC == NULL )
       return;
    struct Mtx_Q *mutx;
  //its a V operation
  //when a V operation is executed a a waiting process if any is waken up and moved to ready state
/*
   if ( typeC->waitQ  == NULL ) {
      
    
      
     return;
    } */
    if ( typeC-> count > MAX_THREADS ) {
       // perror(" no of thread is more than available exiting\n");
       // exit(0);
       return;
      }  
   typeC->count++; 
  
 // printf(" semop count %d \n",typeC->count);
   if ( typeC->count <= 0 ) { //changed here put <=
       
        //WHERE
  //Arange the header
      if (typeC->waitQ->next == NULL) {
         mutx= typeC->waitQ;
         myThreadList[threadCount] = typeC->waitQ->currThread;
         //printf(" the threadlist is %d \n ",typeC->waitQ->currThread.my_thread_id);
         threadCount++;
         free(mutx);
         typeC->waitQ=NULL;
      } else {
      mutx= typeC->waitQ;
      myThreadList[threadCount] = typeC->waitQ->currThread;
      //printf(" the threadlist is %d \n ",typeC->waitQ->currThread.my_thread_id);
      threadCount++;
      typeC->waitQ= typeC->waitQ->next;
      free(mutx);  
      
    }
  }
 //mt_yield();
  return;
 }

void 
  mt_sem_destroy ( mt_sem ksem)
 {
    
   semop *typeC = (semop *)ksem;
   if ( typeC == NULL )
       return;
   free(typeC);
  
}
//sem_down function
void
  mt_sem_down ( mt_sem ksem )
   {
    semop *typeC = (semop *)ksem;
    if ( typeC == NULL )
       return;
    struct Mtx_Q *stemp;
    myThread temp;
  //its a P operation
  //when a P operation is executed a process is added from readyQ to waitQ
  if ( WHERE_AM_I == 0) {
        return;
    }
   typeC->count--;
   if (typeC -> count >= 0) {   
     
    return;
   }
   //Add method
   if (threadCount < (-1) * MAX_THREADS )
   return;  
  
   temp = myThreadList[threadInUse];
   if ( typeC-> count < 0 ) {
  
	   if ( typeC ->waitQ == NULL) { // to take care of head 
	    typeC->waitQ = ( struct Mtx_Q *)malloc(sizeof(struct Mtx_Q));
	    typeC->waitQ->currThread = temp;
	    typeC->waitQ->next = NULL;
	   }
	   else {
           // WHERE
	      stemp=typeC->waitQ;
	      while( typeC -> waitQ -> next != NULL) //checking whether there is 
	      typeC = typeC->waitQ->next;
	      typeC->waitQ->next= ( struct Mtx_Q *)malloc(sizeof(struct Mtx_Q));
	      typeC->waitQ=typeC -> waitQ ->next;
	      typeC->waitQ->currThread = temp;
	      typeC ->waitQ -> next= NULL;
	      typeC ->waitQ = stemp;
	     }
	     threadCount--;
		// if the last thread is used
	  if( threadInUse != threadCount )
	    {
		 myThreadList[threadInUse] = myThreadList[threadCount];
                 //mt_yield();
		 swapcontext( &myThreadList[threadInUse].th_context, &myThreadList[threadCount].th_context);
	     }
             else 
               mt_yield();
	/*  else   {
	      if ( threadInUse != 1 )
		    swapcontext( &myThreadList[threadInUse].th_context, &myThreadList[threadInUse - 1].th_context);
		 } */
      }
  return;
 }

// Add portion for semaphore #####################################################
//this is for thread queue clean up and arrange the queue
void
  ArrangeThreadQueue () 
 {
   free(myThreadList[threadInUse].th_stack); //free the stack
   // we are maintaining a circular thread queue
   // So for that :
    --threadCount;
 // if the last thread is used
  if( threadInUse != threadCount )
    {
       myThreadList[threadInUse] = myThreadList[threadCount];
     }
 myThreadList[threadCount].state = FREE;
// free(myThreadList[threadCount].th_stack);
 }

int
 threadDispatcher() 
 {
  //A main thread is there with thread id , while dispatching we needn to make sure , the threadid 0 will not appear
  int j,i;
 struct timespec tv1;
tv1.tv_sec = 0;	
tv1.tv_nsec = 1;

//printf(" in scheduler total threads %d  state %d waitc %d\n",threadCount,myThreadList[1].state,waitCount); 
 
   //check sleep queue and add it to ready queue
      while (1) {
	  for ( j = 0; j < threadCount ; j++ ) {
          for(i=0;i< 20000;i++);
           threadInUse = ( threadInUse + 1) % threadCount;
           if ( threadInUse == 0 )
           threadInUse = ( threadInUse + 1) % threadCount;
	     if( myThreadList[threadInUse].state == SLEEP )              
	        { 
                   
                  //anosleep(&tv1, NULL);
                 // for(i=0;i< 20000;i++)
                   //  printf("\nit\n");
                 // for(i=0;i< 20000;i++)
                  //    printf("\nit\n");
                      
                  if ( time(0) - myThreadList[threadInUse].time_to_wake_up > 0) {
                    //WHERE
                        myThreadList[threadInUse].state == ACTIVE;
                        waitCount--;
                 // printf("\n in sleep-exit %d \n",threadInUse);
                     return threadInUse;
                   }
                //if ( threadCount == waitCount )
                  //j=0;
                  //continue;
                }
         else
           return threadInUse;
      }
     }
     //return threadInUse;
}

void 
 mt_init(void) 
 {
   int i;
  for(i =0 ; i < MAX_THREADS ; i++ ) {
    myThreadList[i].state = FREE;
  }
  //threadCount = 1;
  //threadInUse = 0 ; // just initialised 
  if(getcontext( &mainContext ) == -1 )
                die("getcontext");
   myThreadList[THREADID].my_thread_id = THREADID;
   //myThreadList[THREADID].th_context.uc_link = 0;
 return;
 }
 // return the thread id from where the thread is called
void* 
 mt_self()
 {
 

// its 0 for the main thread
  return ((void *)myThreadList[threadInUse].my_thread_id);
 }
//its a yield state u can say :  Switches from a thread[x] to main or from main to a thread[x] 
//reference : http://www.evanjones.ca/software/threading.html
//reference  : https://wiki.engr.illinois.edu/display/cs423fa09/MP2
void 
 mt_yield() 
 {
  // static int swapV=0;
	/* If we are in a thread, then switch to the main process */
	if ( WHERE_AM_I )
	{
		                     
		swapcontext( &myThreadList[threadInUse].th_context, &mainContext );            
	} 
	else { // we are in main
	 if ( threadCount == 1) 
	    return; // no point of executing as there is no more thread
      
	 /* Saved the state so call the next mythread scheduler queue */
	threadInUse = threadDispatcher();
       	WHERE_AM_I = 1;
	swapcontext( &mainContext, &myThreadList[threadInUse].th_context);
	WHERE_AM_I = 0;
	//switching to main context again
	if ( myThreadList[threadInUse].state == FREE )
	 {
	   //we need to take care the process will not run again 
         //Added for the mt_join function
          if (myThreadList[threadInUse].ISJOIN) {
           //WHERE
              return;
          }
	     ArrangeThreadQueue();
	  // --threadCount;
	    
	 }
       }
  }
/* Records when the mythread has started and when it is done
so that we know when to free its resources. It is called in the mythread's
context of execution. */
static void 
 my_th_Start( void (*func)(void *) , void *arg )
 {
     // the thread will be active initially
	myThreadList[threadInUse].state = ACTIVE;
	func(arg);
   //if user dont use yield in the main program , it will work serially
	myThreadList[threadInUse].state = FREE;  //changing the state to FREE
	/* Yield control, but because active == 0, this will free the mythread*/       
	mt_yield();
 }
// thread function to create thread and initialise the required things
 void*
  mt_create(void (*func)(void *), void *arg)
    {
      //threadInUse = threadCount;
      if ( threadCount == MAX_THREADS ) 
           {
             MT_DEBUG("\n Max Thread is open , you can't create  a new Thread \n");
             return FAILURE;
           }

    /* Add the new function to the end of the fiber list */
	if(getcontext( &myThreadList[threadCount].th_context ) == -1 )                
                 die("getcontext");


	/* Set the context to a newly allocated stack */
       // Before the call makecontext the uc_stack and uc_link element of the context structure should be initialized. The uc_stack element describes the stack which is used for this context. No two contexts which are used at the same time should use the same memory region for a stack. Source : http://www.gnu.org/s/hello/manual/libc/System-V-contexts.html
	myThreadList[threadCount].th_context.uc_link = &mainContext; //changed
	myThreadList[threadCount].th_stack = malloc( STACK_SIZE ); //allocating size for the stack
	myThreadList[threadCount].th_context.uc_stack.ss_sp = myThreadList[threadCount].th_stack;
	myThreadList[threadCount].th_context.uc_stack.ss_size = STACK_SIZE; //assigning stack size
	myThreadList[threadCount].th_context.uc_stack.ss_flags = 0;
	myThreadList[threadCount].my_thread_id = ++THREADID ; // for retrieving the threadID
        myThreadList[threadCount].ISJOIN = 0; // to mention it is not joined
        myThreadList[threadCount].state = ACTIVE;
	if ( myThreadList[threadCount].th_stack == 0 )
	{
		MT_DEBUG("\nError: Could not allocate stack.\n" );
		return FAILURE;
	}
	//
       
	/* Create the context. The context calls my_th_start func ). */
// From Wikipedia : The makecontext function sets up an alternate thread of control in ucp, which has previously been initialised using getcontext. The th_context.uc_stack member should be pointed to an appropriately sized stack; the constant MAXSTACKSIZE is  used. When ucp is jumped to using setcontext or swapcontext, execution will begin at the entry point to the function pointed to by func, with argc arguments as specified. When func terminates, control is returned to th_context.uc_link

         makecontext( &myThreadList[threadCount].th_context, (void (*)(void)) &my_th_Start, 2, func,arg);
    
//increment threadCount
	int retval = myThreadList[threadCount].my_thread_id;
	threadCount++;

	return ((void *)retval);
}


void 
  mt_joinall()
{
	int myThreadRemaining = 1;
	          //   printf(" total threads %d \n",threadCount); 
         while ( threadCount > myThreadRemaining )
	{
                //printf("\n total threads %d  remaining %d\n",threadCount,myThreadRemaining); 
		mt_yield(); // keep on calling yield;
              
	} 
  
	return ;
}

//Now mark the current thread as dead, explicitly switch into the \
         * scheduler and let it reap the current thread structure; we can't \
         * free it here, or we'd be running on a stack which malloc() 
void 
 mt_exit () 
 {
	          
	//delete the threadinuse from the thread queue
          //printf("\n in exit %d \n",threadInUse);
        if ( threadCount < 1 )
           return;  // if i am doing exit again n again
        threadCount--; 
	if( threadInUse != threadCount )
	    {
	       myThreadList[threadInUse] = myThreadList[threadCount];
	     }
   	 myThreadList[threadCount].state = FREE;
        //free(myThreadList[threadCount].th_stack); //free the stack
         // --threadCount;
         mt_yield(); //call yield for the scheduler to take care
          
}

//it is to kill one thread from another thread 
void 
 mt_kill (void* threadId )
 {
         int i,position , flag = 0 , thID;
         thID = (int *) threadId;
	 if ( thID == 0) // for main 
	  return;
	 if ( thID > threadCount ) // if i want to call my self or the threadid doesnot exist
	   return;
       
       for (  i = 0; i< threadCount ; i++ ) {
           if ( myThreadList[i].my_thread_id == thID ) 
             { flag = 1; position = i; }
            
          }
          if (!flag) return;
	    --threadCount;
	 // if the last thread is used
          
	  if( position != threadCount )
	    {
	       myThreadList[position] = myThreadList[threadCount]; //swapping the threads in ready queue.
	     }
          free(myThreadList[threadCount].th_stack); //free the stack
	 // myThreadList[position].state = FREE;	 
 }
void 
 mt_join ( void *threadId ) 
 {
          int position , flag = 0 , thID;
          int i;
           myThread temp;
          thID = (int *)threadId;
	  /* track for the main process and in case we want to call "join" the thread itself from the thread itself*/
	  if ( thID == 0 )
	  return;
	 
         for ( i = 1; i< threadCount ; i++ ) {
           if ( myThreadList[i].my_thread_id == thID)
             { flag = 1; position = i; }
            
          }
          if (!flag) return;
	
          while ( myThreadList[position].state != FREE ) {
             //WHERE
             //printf(" %d \n", position);
               mt_yield();
            }
                 
          //myThreadList[threadInUse].ISJOIN = 0;
       return;  
  }
 void 
  mt_sleep ( int sleepTime ) //sleepTime in ms
  {

           if ( sleepTime <= 0 ) return; 
           if ( WHERE_AM_I == 0) return;
           myThreadList[threadInUse].state=SLEEP;
	   myThreadList[threadInUse].time_to_wake_up = sleepTime + time(0) ;
          ++waitCount;
           mt_yield();
          
       }

