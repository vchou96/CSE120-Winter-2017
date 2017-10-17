/* mykernel.c: your portion of the kernel
 *
 *	Below are procedures that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).  
 */

#include "aux.h"
#include "sys.h"
#include "mykernel3.h"

#define FALSE 0
#define TRUE 1

/*	A sample semaphore table. You may change this any way you wish.  
 */

static struct {
	int valid;	// Is this a valid entry (was sem allocated)?
	int value;	// value of semaphore
	int numBlocked;
	int blockedProc[MAXPROCS-1];
} semtab[MAXSEMS];
int count;

/*	InitSem () is called when kernel starts up.  Initialize data
 *	structures (such as the semaphore table) and call any initialization
 *	procedures here. 
 */

void InitSem ()
{
	int s;

	/* modify or add code any way you wish */

	for (s = 0; s < MAXSEMS; s++) {		// mark all sems free
		semtab[s].valid = FALSE;
	}

}

/*	MySeminit (p, v) is called by the kernel whenever the system
 *	call Seminit (v) is called.  The kernel passes the initial
 * 	value v, along with the process ID p of the process that called
 *	Seminit.  MySeminit should allocate a semaphore (find a free entry
 *	in semtab and allocate), initialize that semaphore's value to v,
 *	and then return the ID (i.e., index of the allocated entry). 
 */

int MySeminit (int p, int v)
{
	int s;

	/* modify or add code any way you wish */

	for (s = 0; s < MAXSEMS; s++) {
		if (semtab[s].valid == FALSE) {
			break;
		}
	}
	if (s == MAXSEMS) {
		Printf ("No free semaphores\n");
		return (-1);
	}

	semtab[s].valid = TRUE;
	semtab[s].value = v;//v is int parameter originally sent aka 0
	semtab[s].numBlocked=0;
	return (s);//s is index in table
}

/*	MyWait (p, s) is called by the kernel whenever the system call
 *	Wait (s) is called.  
 */

void MyWait (p, s)
	int p;				// process
	int s;				// semaphore
{
	/* modify or add code any way you wish */
		if(--semtab[s].value<0){
			semtab[s].blockedProc[semtab[s].numBlocked]=p;
			semtab[s].numBlocked++;
			Block(p);
			
		}
	
	
}

/*	MySignal (p, s) is called by the kernel whenever the system call
 *	Signal (s) is called.  
 */

void MySignal (p, s)
	int p;				// process
	int s;				// semaphore
{
	/* modify or add code any way you wish */
	int toUnblock;
	semtab[s].value++;
	if(semtab[s].value-1<0){
		toUnblock= semtab[s].blockedProc[0];
		semtab[s].numBlocked--;
		for(int i=1; i<semtab[s].numBlocked+1; i++){
			semtab[s].blockedProc[i-1]=semtab[s].blockedProc[i];
		}
		Unblock(toUnblock);
	}	
	
}

