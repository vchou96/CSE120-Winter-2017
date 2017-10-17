/*	User-level thread system
 *
 */

#include <setjmp.h>
#include <strings.h>
#include "aux.h"
#include "umix.h"
#include "mythreads.h"

static int MyInitThreadsCalled=0;	// 1 if MyInitThreads called, else 0
static int currThread=0; //the current thread
static int prevThread=0;
static int numThreads; //number of threads active
static int lastID=0;
static int waitingThreads=0;
static int threadQ[MAXTHREADS];
static struct thread {			// thread table
	int valid;			// 1 if entry is valid, else 0
	jmp_buf env;			// current context
	jmp_buf cleanslate;
	void (*f)();
	int p;	
} thread[MAXTHREADS];

#define STACKSIZE	65536		// maximum size of thread stack

/*	MyInitThreads () initializes the thread package. Must be the first
 *	function called by any user program that uses the thread package.  
 */

void MyInitThreads ()
{
	int i;

	if (MyInitThreadsCalled) {		// run only once
		Printf ("InitThreads: should be called only once\n");
		Exit ();
	}
	for (i = 0; i < MAXTHREADS; i++) {	// initialize thread table
		thread[i].valid = 0;
		threadQ[i]=-1;
	}

	thread[0].valid = 1;			// initialize thread 0
	currThread=0;
	numThreads=1;
	lastID=0;
	waitingThreads=0;
	//threadQ[0]=-1;
	if(setjmp(thread[0].env)==0){
		memcpy(thread[0].cleanslate, thread[0].env, sizeof(jmp_buf));
	}
	else{
		//Printf("\n HIYA %d", currThread);
		(*(thread[0].f))(thread[0].p);
		MyExitThread();
	}
	for(int i=1; i<=MAXTHREADS; i++){
		char s[i*STACKSIZE];
		//thread[i].valid=0;
		threadQ[i]=-1;
		//save environment using setjmp(thread[i].env)
		if(setjmp(thread[i].env)==0){
			memcpy(thread[i].cleanslate, thread[i].env, sizeof(jmp_buf));
		}
		else{
			//Printf("\n HIYA %d", currThread);
			(*(thread[currThread].f))(thread[currThread].p);
			MyExitThread();
		}
		
	}
	MyInitThreadsCalled = 1;
	
	
}

/*	MyCreateThread (func, param) creates a new thread to execute
 *	func (param), where func is a function with no return value and
 *	param is an integer parameter.  The new thread does not begin
 *	executing until another thread yields to it. 
 */

int MyCreateThread (func, param)
	void (*func)();			// function to be executed
	int param;			// integer parameter
{
	int found=0;
	if (! MyInitThreadsCalled) {
		Printf ("CreateThread: Must call InitThreads first\n");
		Exit ();
	}

	//if already maxthreads active, return -1
	if(numThreads==MAXTHREADS){
		return (-1);	
	}	
	
	//if (setjmp (thread[0].env) == 0) {	// save context of thread 0
	//if (setjmp (thread[currThread].env) == 0){
		/* The new thread will need stack space.  Here we use the
		 * following trick: the new thread simply uses the current
		 * stack, and so there is no need to allocate space.  However,
		 * to ensure that thread 0's stack may grow and (hopefully)
		 * not bump into thread 1's stack, the top of the stack is
		 * effectively extended automatically by declaring a local
		 * variable (a large "dummy" array). This array is never
		 * actually used; to prevent an optimizing compiler from
		 * removing it, it should be referenced.  
		 */

		//char s[STACKSIZE];	// reserve space for thread 0's stack

		//void (*f)() = func;	// f saves func on top of stack
		//int p = param;		// p saves param on top of stack
		//if (((int) &s[STACKSIZE-1]) - ((int) &s[0]) + 1 != STACKSIZE) {
	//		Printf ("Stack space reservation failed\n");
	//		Exit ();
	//	}
		


		for(int i=lastID+1; i<MAXTHREADS; i++){
			if(!thread[i].valid){//if next index in thread table is free
				thread[i].valid = 1;	// mark the entry for the new thread valid
				found=1;

				lastID=i;
				break;
			}
		}

		if(!found){//did not find spot in table yet, loop around
			for(int i=0; i<lastID+1; i++){
				if(thread[i].valid==0){//if next index in thread able is free
					thread[i].valid = 1;	// mark the entry for the new thread valid
					lastID=i;
					found=1;
					break;
				}
			}
		}
		if(!found)
			return(-1);
		memcpy(&thread[lastID].env, &thread[lastID].cleanslate, sizeof(jmp_buf));
		//add to Q
		threadQ[waitingThreads]=lastID;	
		waitingThreads++;
		thread[lastID].f=func;
		thread[lastID].p=param;
		numThreads++;

	//	if(setjmp(thread[lastID].env)==0){ //first return for new thread
	//		longjmp (thread[currThread].env, lastID);	// back to thread 0
	//	}

		//Printf("\n thread %d scheduled %d param %d", currThread, lastID, param);
		/* here when thread 1 is scheduled for the first time */
		//(*f)(p);			// execute func (param)
	//	(*(thread[currThread].f))(thread[currThread].p);
	//	MyExitThread ();		// thread 1 is done - exit
	//}
	return (lastID);		// done, return new thread ID
	
}

/*	MyYieldThread (t) causes the running thread, call it T, to yield to
 *	thread t.  Returns the ID of the thread that yielded to the calling
 *	thread T, or -1 if t is an invalid ID. Example: given two threads
 *	with IDs 1 and 2, if thread 1 calls MyYieldThread (2), then thread 2
 *	will resume, and if thread 2 then calls MyYieldThread (1), thread 1
 *	will resume by returning from its call to MyYieldThread (2), which
 *	will return the value 2.
 */

int MyYieldThread (t)
	int t;				// thread being yielded to
{
	
	//Printf("\n SIGH %d %d", currThread, t);
	if (! MyInitThreadsCalled) {
		Printf ("YieldThread: Must call InitThreads first\n");
		Exit ();
	}

	if (t < 0 || t >= MAXTHREADS) {
		Printf ("YieldThread: %d is not a valid thread ID\n", t);
		return (-1);
	}
	if (! thread[t].valid) {
		Printf ("YieldThread: Thread %d does not exist\n", t);
		return (-1);
	}
	if(t!=currThread){//yielding to self
		//remove t from Q and run that one
		int i;
		for(i=0; i<waitingThreads;i++ ){
			if(threadQ[i]==t){
				break;
			}	
		} 
		for(i; i<waitingThreads-1;i++){
			threadQ[i]=threadQ[i+1];
		}


		if(thread[currThread].valid==1){
			//Printf("\n KAKAK %d %d", currThread, t);
			threadQ[waitingThreads-1]=currThread;//put current thread at end of Q
		}
		else{
			waitingThreads--;
			threadQ[waitingThreads]=-1;
			prevThread=currThread;
			currThread=t;
			longjmp(thread[t].env, prevThread);
		}
	}
		
	
        if (setjmp (thread[currThread].env) == 0) {
                prevThread=currThread;
		currThread=t;
		//Printf("\n %d yielding to  %d\n", prevThread, t);
		//longjmp (thread[t].env, 1);
		longjmp(thread[t].env, prevThread);
        }
	MyInitThreadsCalled = 1;
	//Printf("\n HELLO %d %d", currThread, prevThread);
	//(*(thread[t].f))(thread[t].p);

	return prevThread;
}

/*	MyGetThread () returns ID of currently running thread.  
 */

int MyGetThread ()
{
	
	if (! MyInitThreadsCalled) {
		Printf ("GetThread: Must call InitThreads first\n");
		Exit ();
	}
	return currThread;

}

/*	MySchedThread () causes the running thread to simply give up the
 *	CPU and allow another thread to be scheduled.  Selecting which
 *	thread to run is determined here. Note that the same thread may
 * 	be chosen (as will be the case if there are no other threads).  
 */

void MySchedThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("SchedThread: Must call InitThreads first\n");
		Exit ();
	}
	//determine which thread to yield to
	if(waitingThreads==0){//no other thread to yield to
		//MyYieldThread(currThread);//yield to itself
	//	Printf("\n What if %d %d", currThread, threadQ[0]);
		
	//	MyExitThread();
	}
	else{
		MyYieldThread(threadQ[0]);//maybe can just call this
	}
		

}

/*	MyExitThread () causes the currently running thread to exit.  
 */

void MyExitThread ()
{

	thread[currThread].valid=0;
	numThreads--;
	
	//if no other threads to pass control to, call exit
	if (! MyInitThreadsCalled) {
		Printf ("ExitThread: Must call InitThreads first\n");
		Exit ();
	}	
	if(waitingThreads){ //if there are threads waiting, call myschedthread
//		Printf("\n end %d", currThread);	
		MySchedThread();
	}
	else{//if no other threads to pass control to, call exit
		Exit();
	}
}
