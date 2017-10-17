#include "aux.h"
#include "umix.h"
#include "mythreads.h"

#define NUMYIELDS   5

static int square, cube;    // global variables, shared by threads

void Main ()
{
    int i, t, me; 
    void printSquares (), printCubes (), printSquare_again();

    MyInitThreads();

    me = MyGetThread();
    t = MyCreateThread(printSquares, me);    

t = MyCreateThread(printCubes, t); 

    for (i = 0; i < NUMYIELDS; i++) {
        //0 yields to 2
        MyYieldThread(0);
        Printf ("T%d: square = %d, cube = %d\n", me, square, cube);
    }   

    MyExitThread (); 
}



void printSquares_again (t) 
    int t;              // thread to yield to
{
    int i;
    //1 yields to 0
    for (i = 0; i < NUMYIELDS; i++) {
        square = i * i;
        Printf ("T%d: %d squared_again = %d\n", MyGetThread (), i, square);
        MyYieldThread (t);
    }   
}


void printSquares (t) 
    int t;              // thread to yield to
{
    int i;
    //1 yields to 0
    for (i = 0; i < NUMYIELDS; i++) {
        square = i * i;
        Printf ("T%d: %d squared = %d\n", MyGetThread (), i, square);
        MyYieldThread (t);
        MyCreateThread(printSquares_again, i+1);
    }   
}

void printCubes (t) 
    int t;              // thread to yield to
{
    int i;

    //2 yields to 1
    for (i = 0; i < NUMYIELDS; i++) {
        cube = i * i * i;
        Printf ("T%d: %d cubed = %d\n", MyGetThread (), i, cube);
        Printf("Print Cubes: thread 2 yields to %d \n", t); 
        MyYieldThread (t);
        MyCreateThread(printSquares, i+1);
    }   
}
