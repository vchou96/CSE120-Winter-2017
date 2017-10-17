/*	User-level thread system
 */

#define MAXTHREADS	10			// maximum number of threads

void MyInitThreads ();				// initialize thread package
int MyCreateThread (void (*f)(), int p);	// create a thread to run f(p)
int MyGetThread ();				// return id of current thread
int MyYieldThread (int t);			// yield to thread t
void MySchedThread ();				// yield to scheduler
void MyExitThread ();				// exit this thread

/*	Working version of above functions (reference user-level thread system)
 */

extern void InitThreads ();			// initialize thread package
extern int CreateThread (void (*f)(), int p);	// create a thread to run f(p)
extern int GetThread ();			// return ID of current thread
extern int YieldThread (int t);			// yield to thread t
extern void SchedThread ();			// yield to scheduler
extern void ExitThread ();			// exit this thread
