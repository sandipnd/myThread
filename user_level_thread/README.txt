This is an assignment for own pthread and use it to dining philosopher problem.

Reference for code is taken from : 

//reference : http://www.evanjones.ca/software/threading.html
//reference  : https://wiki.engr.illinois.edu/display/cs423fa09/MP2


Assumptions:

1) mt_sem_down,sleep called from main will be ignored


Bugs : 

1) if we run test_mt.c program over mythread library , 
  mt_create(Sleep_T1,NULL);
	mt_create(Sleep_T2,NULL);

 >>>>>>. Where every function calls mt_exit at end, after both function is awake from sleep , only the 1st function executes. It looks like a problem with my mt_exit . Not able to fix.
         if we dont use mt_exit at end , it runs fine for any number of sleep function.
 like :
   I'm a sleepy task and I'm sleeping for 3 seconds
  I'm a sleepy task and I'm sleeping for 5 seconds
  yielder 1 ending
  yielder 2 ending
  I'm a sleepy task 3 and I'm awake on time
  exit

2) It may be a cosmetic issue , a scheduler glitch may be. scheduler schedules what job comes first and in what sequence . 
 Assumptions: mt_sem_down call from main will be ignored.


 if we call like this sequence:
  t1 = mt_create(Thread_4,(void *)s2);

	fprintf(stdout,"main forked Thread_4 and calling P on s1\n");
	fflush(stdout);

	mt_sem_down(s1);

	fprintf(stdout,"main out of first P call\n");
	fflush(stdout);

	t2 = mt_create(Thread_5,(void *)s2);

we will get this :
 I'm thread 5 and I'm synching with thread 4
I'm thread 5 and I'm back from synching with thread 4
I'm thread 4 and I'm synching with thread 5
I'm thread 4 and I'm back from synching with thread 5

the output will be different if we interchange t1 and t2  : it completely depends on calling sequence

  

  
i have not refered dphil.h , functions are integrated in the dphil_skeleton.c ( the file is modified according to the requirement)
  
reference (dining-philosopher)http://thecodecracker.com/c-programming/dining-philosophers-problem/
