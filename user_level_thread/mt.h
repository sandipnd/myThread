#if !defined(MT_H)
#define MT_H

#ifdef SOLARIS
#define JB_SP (1)
#define JB_FP (3)
#endif

#ifdef LINUX
#define JB_BP    3    
#define JB_SP    4    
#endif

#ifdef MACOSX
#define JB_SP 0
/* don't bother with FP */
#endif
#define WHERE fprintf(stdout,"%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
/*
 * mt_init() initializes the library
 */ 
extern void mt_init(void);

/*
 * returns my thread id
 */
extern void *mt_self();


/*
 * mt_create() creates a new, runnable thread
 *
 * func: entry point for the new thread
 * arg: single argument to be passed
 */
extern void* mt_create(void (*func)(void *), void *arg);

/*
 * mt_join() blocks waiting for the completion of the thread specified
 *           in its argument list
 *
 * mt: the thread to wait for
 */
extern void mt_join(void *mt);

/*
 * mt_joinall() blocks until there are not runnable threads left
 */
extern void mt_joinall();

/*
 * mt_exit() kills the calling thread immediately
 */
extern void mt_exit();

/*
 **** Semaphore entry points start here
 */
typedef void *mt_sem;

/*
 * make_mt_sem() creates and initializes with its first argument which must
 *               be >= 0
 *               
 * initval: initial semaphore value
 */
mt_sem mt_sem_create(int initval);

/*
 * kill_mt_sem() deallocates the semaphore.  If there are threads blocked on
 *               the semaphore, kill_mt_sem() will cause the program to exit.
 *
 * ksem: semaphore to deallocate
 */
void mt_sem_destroy(mt_sem ksem);

/*
 * mt_getval(mt_sem s) retursn the current value
 */

extern int mt_sem_getval(mt_sem s);

/*
 * mt_sem_down() performs the P semaphore operation.  The current value is
 *            decremented and if it is < 0, the calling thread is queued on
 *            the semaphore and blocked
 *
 * ksem: relevant semaphore
 */
extern void mt_sem_down(mt_sem ksem);

/*
 * mt_sem_up() performs the V operation.  The value is incremented.  If it is 
 *            <= 0, a thread is dequeued and awakened.
 *
 * ksem: relevant semaphore
 */
extern void mt_sem_up(mt_sem ksem);

/*
 * mt_sleep(t) puts the thread to sleep for t seconds
 */
extern void mt_sleep(int secs);

/*
 * mt_yield() abdicates the CPU to that other runnable threads may execute

 */
extern void mt_yield();

/*
 * mt_kill() allows one thread to kill another
 */
extern  void mt_kill(void *t);

#endif

