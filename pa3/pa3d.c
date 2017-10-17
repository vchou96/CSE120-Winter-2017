/* Programming Assignment 3: Exercise D
 *
 * Now that you have a working implementation of semaphores, you can
 * implement a more sophisticated synchronization scheme for the car
 * simulation.
 *
 * Study the program below.  Car 1 begins driving over the road, entering
 * from the East at 40 mph.  After 900 seconds, both Car 3 and Car 4 try to
 * enter the road.  Car 1 may or may not have exited by this time (it should
 * exit after 900 seconds, but recall that the times in the simulation are
 * approximate).  If Car 1 has not exited and Car 4 enters, they will crash
 * (why?).  If Car 1 has exited, Car 3 and Car 4 will be able to enter the
 * road, but since they enter from opposite directions, they will eventually
 * crash.  Finally, after 1200 seconds, Car 2 enters the road from the West,
 * and traveling twice as fast as Car 4.  If Car 3 was not coming from the
 * opposite direction, Car 2 would eventually reach Car 4 from the back and
 * crash.  (You may wish to experiment with reducing the initial delay of
 * Car 2, or increase the initial delay of Car 3, to cause Car 2 to crash
 * with Car 4 before Car 3 crashes with Car 4.)
 *
 *
 * Exercises
 *
 * 1. Modify the procedure driveRoad such that the following rules are obeyed:
 *
 *	A. Avoid all collisions.
 *
 *	B. Multiple cars should be allowed on the road, as long as they are
 *	traveling in the same direction.
 *
 *	C. If a car arrives and there are already other cars traveling in the
 *	SAME DIRECTION, the arriving car should be allowed enter as soon as it
 *	can. Two situations might prevent this car from entering immediately:
 *	(1) there is a car immediately in front of it (going in the same
 *	direction), and if it enters it will crash (which would break rule A);
 *	(2) one or more cars have arrived at the other end to travel in the
 *	opposite direction and are waiting for the current cars on the road
 *	to exit, which is covered by the next rule.
 *
 *	D. If a car arrives and there are already other cars traveling in the
 *	OPPOSITE DIRECTION, the arriving car must wait until all these other
 *	cars complete their course over the road and exit.  It should only wait
 *	for the cars already on the road to exit; no new cars traveling in the
 *	same direction as the existing ones should be allowed to enter.
 *
 *	E. This last rule implies that if there are multiple cars at each end
 *	waiting to enter the road, each side will take turns in allowing one
 *	car to enter and exit.  (However, if there are no cars waiting at one
 *	end, then as cars arrive at the other end, they should be allowed to
 *	enter the road immediately.)
 *	
 *	F. If the road is free (no cars), then any car attempting to enter
 *	should not be prevented from doing so.
 *
 *	G. All starvation must be avoided.  For example, any car that is
 *	waiting must eventually be allowed to proceed.
 *
 * This must be achieved ONLY by adding synchronization and making use of
 * shared memory (as described in Exercise C).  You should NOT modify the
 * delays or speeds to solve the problem.  In other words, the delays and
 * speeds are givens, and your goal is to enforce the above rules by making
 * use of only semaphores and shared memory.
 *
 * 2. Devise different tests (using different numbers of cars, speeds,
 * directions) to see whether your improved implementation of driveRoad
 * obeys the rules above.
 *
 * IMPLEMENTATION GUIDELINES
 * 
 * 1. Avoid busy waiting. In class one of the reasons given for using
 * semaphores was to avoid busy waiting in user code and limit it to
 * minimal use in certain parts of the kernel. This is because busy
 * waiting uses up CPU time, but a blocked process does not. You have
 * semaphores available to implement the driveRoad function, so you
 * should not use busy waiting anywhere.
 *
 * 2. Prevent race conditions. One reason for using semaphores is to
 * enforce mutual exclusion on critical sections to avoid race conditions.
 * You will be using shared memory in your driveRoad implementation.
 * Identify the places in your code where there may be race conditions
 * (the result of a computation on shared memory depends on the order
 * that processes execute).  Prevent these race conditions from occurring
 * by using semaphores.
 *
 * 3. Implement semaphores fully and robustly.  It is possible for your
 * driveRoad function to work with an incorrect implementation of
 * semaphores, because controlling cars does not exercise every use of
 * semaphores.  You will be penalized if your semaphores are not correctly
 * implemented, even if your driveRoad works.
 *
 * 4. Control cars with semaphores: Semaphores should be the basic
 * mechanism for enforcing the rules on driving cars. You should not
 * force cars to delay in other ways inside driveRoad such as by calling
 * the Delay function or changing the speed of a car. (You can leave in
 * the delay that is already there that represents the car's speed, just
 * don't add any additional delaying).  Also, you should not be making
 * decisions on what cars do using calculations based on car speed (since
 * there are a number of unpredictable factors that can affect the
 * actual cars' progress).
 *
 * GRADING INFORMATION
 *
 * 1. Semaphores: We will run a number of programs that test your
 * semaphores directly (without using cars at all). For example:
 * enforcing mututal exclusion, testing robustness of your list of
 * waiting processes, calling signal and wait many times to make sure
 * the semaphore state is consistent, etc.
 *
 * 2. Cars: We will run some tests of cars arriving in different ways,
 * to make sure that you correctly enforce all the rules for cars given
 * in the assignment.  We will use a correct semaphore implementation for
 * these tests so that even if your semaphores are not correct you could
 * still get full credit on the driving part of the grade.  Think about
 * how your driveRoad might handle different situations and write your
 * own tests with cars arriving in different ways to make sure you handle
 * all cases correctly.
 *
 *
 * WHAT TO TURN IN
 *
 * You must turn in two files: mykernel3.c and p3d.c.  mykernel3.c should
 * contain you implementation of semaphores, and p3d.c should contain
 * your modified version of InitRoad and driveRoad (Main will be ignored).
 * Note that you may set up your static shared memory struct and other
 * functions as you wish. They should be accessed via InitRoad and driveRoad,
 * as those are the functions that we will call to test your code.
 *
 * Your programs will be tested with various Main programs that will exercise
 * your semaphore implementation, AND different numbers of cars, directions,
 * and speeds, to exercise your driveRoad function.  Our Main programs will
 * first call InitRoad before calling driveRoad.  Make sure you do as much
 * rigorous testing yourself to be sure your implementations are robust.
 */

#include <stdio.h>
#include "aux.h"
#include "sys.h"
#include "umix.h"

void InitRoad (void);
void driveRoad (int from, int mph);

static struct{
	int numWest;//actually on road
	int numEast;
	int roadPos[NUMPOS+2];//value of semaphore
	int blockedPos[NUMPOS+2];//how many process blocked in each pos
} shm;


void Main ()
{
	InitRoad ();

	/* The following code is specific to this particular simulation,
	 * e.g., number of cars, directions, and speeds.  You should
	 * experiment with different numbers of cars, directions, and
	 * speeds to test your modification of driveRoad.  When your
	 * solution is tested, we will use different Main procedures,
	 * which will first call InitRoad before any calls to driveRoad.
	 * So, you should do any initializations in InitRoad.
	 */

	if (Fork () == 0) {
		Delay (0);			// car 2
		driveRoad (EAST, 10);
		Exit ();
	}

	if (Fork () == 0) {
		Delay (0);			// car 3
		driveRoad (EAST, 20);
		Exit ();
	}

	if (Fork () == 0) {
		Delay (0);			// car 4
		driveRoad (EAST, 30);
		Exit ();
	}

	if (Fork () == 0) {
		Delay (0);			// car 5
		driveRoad (EAST, 40);
		Exit ();
	}
	

	if (Fork () == 0) {
		Delay (0);			// car 6
		driveRoad (EAST, 50);
		Exit ();
	}
	
	if (Fork () == 0) {
		Delay (0);			// car 7;
		driveRoad (EAST, 60);
		Exit ();
	}
	
	if (Fork () == 0) {
		Delay (0);			// car 7;
		driveRoad (EAST, 70);
		Exit ();
	}
	
	if (Fork () == 0) {
		Delay (0);			// car 7;
		driveRoad (EAST, 80);
		Exit ();
	}
	
	if (Fork () == 0) {
		Delay (0);			// car 7;
		driveRoad (EAST, 90);
		Exit ();
	}
	
	driveRoad (EAST,5);			// car 1

	Exit ();
}

/* Our tests will call your versions of InitRoad and driveRoad, so your
 * solution to the car simulation should be limited to modifying the code
 * below.  This is in addition to your implementation of semaphores
 * contained in mykernel3.c.
 */

void InitRoad ()
{
	/* do any initializations here */
	Regshm((char *) &shm, sizeof(shm));
	for(int i=0; i<NUMPOS+2; i++){//initialize all 10 semaphores
		shm.roadPos[i]=Seminit(0)-i;
	}
	
	
	
}

#define IPOS(FROM)	(((FROM) == WEST) ? 1 : NUMPOS)

void driveRoad (from, mph)
	int from;			// coming from which direction
	int mph;			// speed of car
{
	int c;				// car ID c = process ID
	int p, np, i;			// positions

	c = Getpid ();			// learn this car's ID

	if(from==0){//from West
		if(shm.numEast>0 || shm.blockedPos[NUMPOS+1]>0){//wait on sema #11 since cars coming from East or one already waiting on other side at sema #12
			shm.blockedPos[NUMPOS]++;//increment sema #11 blocked count
			Wait(NUMPOS+1);//add to sema #11 wait queue
			shm.blockedPos[NUMPOS]--;
		}
		else if(shm.roadPos[0]<0){//if West cars going and one blocking sema #1
			shm.blockedPos[0]++;//increment sema #1 blocked count
			Wait(1);//add to sema #1 wait queue
			shm.blockedPos[0]--;
			
		}
		shm.numWest++;//ready to go on road
	}
	else{//from East
		if(shm.numWest>0 ||shm.blockedPos[NUMPOS]>0){//wait on sema #12 since cars coming from West or one already waiting on other side at sema #11
			shm.blockedPos[NUMPOS+1]++;//increment sema #12 blocked count
			
			Wait(NUMPOS+2);//add to sema #12 wait queue
			
			shm.blockedPos[NUMPOS+1]--;
		}
		else if(shm.roadPos[NUMPOS-1]<0){//if East cars going and one blocking sema #10  
			shm.blockedPos[NUMPOS-1]++;//increment sema #10 blocked count
			
			Wait(NUMPOS);//add to sema #10 wait queue
			shm.blockedPos[NUMPOS-1]--;
			
		}
		shm.numEast++;//ready to go on road
	}
	EnterRoad (from);
	PrintRoad ();
	Printf ("Car %d enters at %d at %d mph\n", c, IPOS(from), mph);
	
	for (i = 1; i < NUMPOS; i++) {
		if (from == WEST) {
			p = i;
			np = i + 1;
		} else {
			p = NUMPOS + 1 - i;
			np = p - 1;
		}
		--shm.roadPos[p-1];//takes up current position
		Delay (3600/mph);
		
		if(shm.roadPos[np-1]<0){//something ahead cannot proceed
			++shm.blockedPos[np-1];//increment next sema blocked count
			Wait(np);//wait on next semaphore
			--shm.blockedPos[np-1];
		}
			
		
		++shm.roadPos[p-1];//leaves current pos free
		ProceedRoad ();
		PrintRoad ();
		Printf ("Car %d moves from %d to %d\n", c, p, np);
		if(shm.blockedPos[p-1]>0){//if old pos blocked a process
				Signal(p);
		}
		if(i==1 && from==0 && shm.blockedPos[NUMPOS]>0 && shm.blockedPos[NUMPOS+1]==0)//from West, if more from West waiting and none from East waiting
			Signal(NUMPOS+1);
		else if (i==1 && from==1 && shm.blockedPos[NUMPOS+1]>0 && shm.blockedPos[NUMPOS]==0)//from East, if more from East waiting and none from West waiting 
			Signal(NUMPOS+2);
	}
		
	--shm.roadPos[np-1];//takes up current position
	Delay (3600/mph);
	ProceedRoad ();
	PrintRoad ();
	Printf ("Car %d exits road\n", c);
	++shm.roadPos[np-1];//leaves current pos free
	if (from==0){//from West
		--shm.numWest;//take off road
		if(shm.blockedPos[NUMPOS-1]>0){//if old pos blocked a process
			Signal(np);
		}
		if(shm.blockedPos[NUMPOS+1]>0 && shm.numWest==0)//if no more cars from West & some waiting on sema 12
			Signal(NUMPOS+2);
	}
	else{
		--shm.numEast;//take off road
		if(shm.blockedPos[0]>0)//if old pos blocked a process 
			Signal(np);
		if(shm.blockedPos[NUMPOS]>0 && shm.numEast==0)//if no more cars from East and some waiting on 11
			Signal(NUMPOS+1);
	}
	
}
