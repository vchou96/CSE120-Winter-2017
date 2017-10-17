/* Copyright 1989-2013, Joseph Pasquale, University of California, San Diego
 *
 *	mykernel interface for pa3
 */

void InitSem ();			/* initialize semaphores */
int MySeminit (int p, int v);		/* alloc sem, init to v, return semid */
void MyWait (int p, int s);		/* process p waits using sem s */
void MySignal (int p, int s);		/* process p signals using sem s */
