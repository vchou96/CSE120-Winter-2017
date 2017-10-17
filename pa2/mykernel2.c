/* mykernel2.c: your portion of the kernel
 *
 *	Below are procedures that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).  
 * 
 */

#include "aux.h"
#include "sys.h"
#include "mykernel2.h"

#define TIMERINTERVAL 1	// in ticks (tick = 10 msec)
static int clock=0;
int current=0;
int numprocess=0;
float cputaken=0;
float unrequested=100;
int numofunreq=0;

/*	A sample process table. You may change this any way you wish.  
 */

static struct {
	int timestamp;
	int valid;		// is this entry valid: 1 = yes, 0 = no
	int pid;		// process ID (as provided by kernel)
	int request;
	int percent;
	int stride;
	int pass;
} proctab[MAXPROCS];


/*	InitSched () is called when kernel starts up.  First, set the
 *	scheduling policy (see sys.h). Make sure you follow the rules
 *	below on where and how to set it.  Next, initialize all your data
 *	structures (such as the process table).  Finally, set the timer
 *	to interrupt after a specified number of ticks. 
 */

void InitSched ()
{
	int i;
	/* First, set the scheduling policy.  You should only set it
	 * from within this conditional statement.  While you are working
	 * on this assignment, GetSchedPolicy will return NOSCHEDPOLICY,
	 * and so the condition will be true and you may set the scheduling
	 * policy to whatever you choose (i.e., you may replace ARBITRARY). 
	 * After the assignment is over, during the testing phase, we will
	 * have GetSchedPolicy return the policy we wish to test, and so
	 * the condition will be false and SetSchedPolicy will not bei
	 * called, thus leaving the policy to whatever we chose to test.  
	 */
	if (GetSchedPolicy () == NOSCHEDPOLICY) {	// leave as is
		//SetSchedPolicy (ARBITRARY);		// set policy here
		SetSchedPolicy (FIFO);
	}

	/* Initialize all your data structures here */
	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;
	}
	
	/* Set the timer last */
	SetTimer (TIMERINTERVAL);
}


/*	StartingProc (pid) is called by the kernel when the process
 *	identified by pid is starting.  This allows you to record the
 *	arrival of a new process in the process table, and allocate
 *	any resources (if necessary). Returns 1 if successful, 0 otherwise.  
 */

int StartingProc (pid)
	int pid;			// process that is starting
{
	int i;
	int found=0;
	

	if(GetSchedPolicy()==LIFO)
		DoSched();
	for (i = 0; i < MAXPROCS; i++) {
		if (! proctab[i].valid) {
			found=1;
			proctab[i].valid = 1;
			proctab[i].pid = pid;
			proctab[i].timestamp=clock;
			proctab[i].percent=0;
			proctab[i].pass=0;
			proctab[i].stride=100000;
			proctab[i].request=0;
			clock++;
			numofunreq++;
			numprocess++;
			break;
		}
	}
	for(int i=0; i<MAXPROCS; i++){
		if(proctab[i].valid==1){
			proctab[i].pass=0;
			if(unrequested>0){
				if(proctab[i].request==0){
					proctab[i].percent=unrequested/numofunreq;
					proctab[i].stride= 100000/proctab[i].percent;
				}
			}
		}
	}
	
	if(found==1)
		return(1);
	
	Printf ("Error in StartingProc: no free table entries\n");
	return (0);
}
			

/*	EndingProc (pid) is called by the kernel when the process
 *	identified by pid is ending.  This allows you to update the
 *	process table accordingly, and deallocate any resources (if
 *	necessary). Returns 1 if successful, 0 otherwise.  
 */


int EndingProc (pid)
	int pid;			// process that is ending
{
	int i;
	int found=0;
	for (i = 0; i < MAXPROCS; i++) {
		if (proctab[i].valid){
			proctab[i].pass=0;
			if (proctab[i].pid == pid) {
				found=1;
				proctab[i].stride=100000;
				if(proctab[i].request==1)
					proctab[i].request=0;
				else
					numofunreq--;
				proctab[i].valid = 0;
				cputaken=cputaken-proctab[i].percent;
				unrequested= 100-cputaken;
				proctab[i].percent=0;
				numprocess--;
			}
		}
	}
	if(unrequested>0){
		for(int i=0; i<MAXPROCS; i++){
			if(proctab[i].valid){
				if(proctab[i].request==0){
					proctab[i].percent=unrequested/numofunreq;
					proctab[i].stride= 100000/proctab[i].percent;
				}
			}
		}
	}
	if (found==1)
		return(1);
	Printf ("Error in EndingProc: can't find process %d\n", pid);
	return (0);
}


/*	SchedProc () is called by kernel when it needs a decision for
 *	which process to run next.  It calls the kernel function
 *	GetSchedPolicy () which will return the current scheduling policy
 *	which was previously set via SetSchedPolicy (policy). SchedProc ()
 *	should return a process id, or 0 if there are no processes to run.  
 */

int SchedProc ()
{
	int i;
	int novalids=0;
	int nopass=0;
	int first_process;
	int curr_valid=0;
	int new_timestamp;
	int small_pass= 100001;
	switch (GetSchedPolicy ()) {

	case ARBITRARY:

		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				return (proctab[i].pid);
			}
		}
		break;

	case FIFO:
		first_process=MAXPROCS;
		new_timestamp=clock;
		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				novalids=1;
				if(proctab[i].timestamp < new_timestamp){
					new_timestamp= proctab[i].timestamp;
					first_process= proctab[i].pid;
				}
			}
		}
		if (novalids==0)
			return 0;
		return first_process;
		
		

		break;

	case LIFO:	
		first_process=MAXPROCS;
		new_timestamp=0;
		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				novalids=1;
				if(proctab[i].timestamp >= new_timestamp){
					new_timestamp= proctab[i].timestamp;
					first_process= proctab[i].pid;
				}
			}
		}
		if (novalids==0)
			return 0;
		return first_process;
		

		break;

	case ROUNDROBIN:

		for (i = current; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				novalids=1;
				current++;
				return proctab[i].pid;
			}
			
		}
		for(i=0; i< current; i++){
			if (proctab[i].valid) {
				novalids=1;
				current=i+1;
				return proctab[i].pid;
			}
		}
		
		if (novalids==0)
			return 0;

		break;

	case PROPORTIONAL:
		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
			novalids=1;
			if(proctab[i].pass < small_pass && proctab[i].percent>0){
						small_pass= proctab[i].pass;
						first_process= proctab[i].pid;
						current=i;
					
				}
			}

		}
		if (novalids==0)
			return 0;

		for(i=0; i<MAXPROCS; i++){
			if (proctab[i].valid)
				proctab[i].pass=proctab[i].pass-small_pass; 
		}
		proctab[current].pass= proctab[current].pass+proctab[current].stride;
		return first_process;
		break;

	}
	
	return (0);
}


/*	HandleTimerIntr () is called by the kernel whenever a timer
 *	interrupt occurs.  
 */

void HandleTimerIntr ()
{
	SetTimer (TIMERINTERVAL);

	switch (GetSchedPolicy ()) {	// is policy preemptive?

	case ROUNDROBIN:		// ROUNDROBIN is preemptive
		if(numprocess!=0){
			DoSched();
		}
		break;
	case PROPORTIONAL:		// PROPORTIONAL is preemptive
		if(numprocess!=0)
			DoSched ();		// make scheduling decision
		break;

	default:			// if non-preemptive, do nothing
		break;
	}
}

/*	MyRequestCPUrate (pid, n) is called by the kernel whenever a process
 *	identified by pid calls RequestCPUrate (n). This is a request for
 *	n% of CPU time, i.e., requesting a CPU whose speed is effectively
 *	n% of the actual CPU speed.  Roughly n out of every 100 quantums
 *	should be allocated to the calling process.  n must be greater than
 *	0 and must be less than or equal to 100. MyRequestCPUrate (pid, n)
 *	should return 0 if successful, i.e., if such a request can be
 *	satisfied, otherwise it should return -1, i.e., error (including if
 *	n < 1 or n > 100).  If MyRequestCPUrate (pid, n) fails, it should
 *	have no effect on scheduling of this or any other process, i.e., AS
 *	IF IT WERE NEVER CALLED.
 */

int MyRequestCPUrate (pid, n)
	int pid;			// process whose rate to change
	int n;				// percent of CPU time
{
	int i;
	if (n<1 || n>100)
		return -1;
	if(n+cputaken>100)
		return -1;
	for (i = 0; i < MAXPROCS; i++) {
		if (proctab[i].valid && pid==proctab[i].pid) 
			break;
	}
	proctab[i].request=1;
	proctab[i].percent=n;	
	proctab[i].stride=100000/n;
	proctab[i].pass=0;
	cputaken=cputaken+n;
	unrequested=100-cputaken;
	numofunreq--;
	for(int j=0; j<MAXPROCS; j++){
		if(proctab[j].valid){
			proctab[j].pass=0;
			if(proctab[j].request==0){
				if(unrequested==0){
					proctab[j].percent=0;
					proctab[j].stride=100000;
				}
				else{
					proctab[j].percent=unrequested/numofunreq;
					proctab[j].stride= 100000/proctab[j].percent;
				}
			}
		}
	}

	return (0);
}
