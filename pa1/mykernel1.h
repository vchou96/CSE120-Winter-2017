/* Copyright 1989-2017, Joseph Pasquale, University of California, San Diego
 *
 *	mykernel interface for pa1
 */

void NewContext (int p, CONTEXT *c);	// initial context for new process p
int MySwitchContext (int p);		// context switch to process p
