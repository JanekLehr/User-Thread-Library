/* 
/ Authors: Janek Lehr and Nick Blazier
/ February 10th, 2012
/ This is a basic thread library for user level threads. It has three main funtionalities:
/ -Create a thread
/ -Yield to a thread
/ -Destroy a thread.
*/


#include <assert.h>
#include <stdio.h>

/* We want the extra information from these definitions */
#ifndef __USE_GNU
#define __USE_GNU
#endif /* __USE_GNU */
#include <stdlib.h>
#include <ucontext.h>
#include "ULT.h"

Tid ULT_DestroyThread(Tid tid);
int validTid(Tid toTest);
int findNextTid();

int readySize;
int firstTime = 1;

// Taken from project specs.
// param root is the starting function for a thread.
// param arg are the arguments passed to the root function..
void stub(void (*root)(void *), void *arg) 
{
	// thread starts here 
	Tid ret; 
	root(arg); // call root function 
	ret = ULT_DestroyThread(ULT_SELF);
	assert(ret == ULT_NONE); // we should only get here if we are the last thread. 
	exit(0); // all threads are done, so process should exit

}

// Remove thread from the ready queue based on Tid.
// param thread is the Tid of the thread to be removed from the readyQueue.
// returns the Thread Control Block that holds the information of the requested thread.
ThrdCtlBlk *removeFromReady(Tid thread)
{
	int i;
	ThrdCtlBlk *b;
	for(i=0; i<readySize; i++)
	{
		if( (readyQueue[i]->threadID) == thread){
			b = readyQueue[i];
			break;
		}
	}
	for(;i<readySize - 1; i++){
		readyQueue[i] = readyQueue[i+1];
	}
	readyQueue[readySize-1] = NULL;
	readySize--;
	return b;
}

// Removes and returns the frist ThrdCtlBlock from readyQueue
// returns the Thread COntrol Block of the first thread in the readyQueue.
ThrdCtlBlk *dequeue()
{
	ThrdCtlBlk *b = readyQueue[0];
	int i;
	for(i = 0; i<readySize - 1; i++){
		readyQueue[i] = readyQueue[i+1];
	}
	readyQueue[readySize-1] = NULL;
	readySize--;
	return b;
}

// This is called the first time ULT is used.
// Initializes the runningThread as the first thread with Tid 0.
// Sets the size of the readyQueue.
// returns 1 on success.
int ULT_Initialize()
{
    firstTime = 0;
	runningThread = (ThrdCtlBlk *)malloc(sizeof(ThrdCtlBlk));
	ThrdCtlBlk *b = (ThrdCtlBlk *)malloc(sizeof(ThrdCtlBlk));
	ucontext_t current;
	getcontext(&current);
	
	b->context = (ucontext_t *)malloc(sizeof(ucontext_t));
	b->context = &current;
	
	b->threadID = 0;
	readySize = 0;
	runningThread = b;
	
	return 1;
}

// Create a thread with fn as the start function with parameters 
// param fn is the starting function of the thread.
// param parg are the arguments to be used by fn.
// returns the Tid of the created thread.
Tid ULT_CreateThread(void (*fn)(void *), void *parg)
{
	if(firstTime)
		ULT_Initialize();

	if(readySize + 1 == ULT_MAX_THREADS)
		return ULT_NOMORE;

	ucontext_t *con = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(con);

	unsigned int *stackPointer = malloc(ULT_MIN_STACK*sizeof(unsigned int));
	stackPointer[ULT_MIN_STACK-1] = (unsigned int)parg;
	stackPointer[ULT_MIN_STACK-2] = (unsigned int)fn;

	con->uc_mcontext.gregs[REG_EIP] = (unsigned int)&stub;
	con->uc_mcontext.gregs[REG_ESP] = (unsigned int)&stackPointer[ULT_MIN_STACK - 3];

	ThrdCtlBlk *newThread = malloc(sizeof(ThrdCtlBlk));
	newThread->context = con;
	newThread->threadID = findNextTid();

	readyQueue[readySize] = newThread;
	readySize++;

	return newThread->threadID;
}


// Passes control to the thread requested and puts the currently running thread on the readyQueue.
// param wantTid is the Tid of the thread that will gain control.
// returns the Tid of the thread that gained control.
Tid ULT_Yield(Tid wantTid)
{
	if(firstTime)
		ULT_Initialize();

	//If it wants self or any and there are no other threads
	if((wantTid == ULT_SELF || wantTid == ULT_ANY) && readySize == 0)
		return ULT_NONE;

	//If it wants self and there are others return self
	if(wantTid == ULT_SELF)
		return runningThread->threadID;

	//If asking for any, switch to the first thread on the queue
	if(wantTid == ULT_ANY){
	  
		int flag = 0;
		ThrdCtlBlk *nextThread;
		nextThread = dequeue();
		getcontext(runningThread->context);
		// Make sure that we don't run this code if we are resuming a thread.
		if(flag == 0){
			readyQueue[readySize] = runningThread;
			readySize++;
			flag = 1;
			runningThread = nextThread;
			setcontext(runningThread->context);
		}

		return nextThread->threadID;
	}

	if(wantTid == runningThread->threadID)
		return runningThread->threadID; 

	if(!validTid(wantTid))
		return ULT_INVALID;

	// Yield to a specified thread instead of ULT_ANY
	int flag = 0;
	ThrdCtlBlk *nextThread;
	nextThread = removeFromReady(wantTid);
	ucontext_t *running = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(running);
	runningThread->context = running;
	if( flag == 0){
		readyQueue[readySize] = runningThread;
		readySize++;
		flag = 1;
		runningThread = nextThread;
		setcontext(runningThread->context);
	}
	return nextThread->threadID;
}

// returns a valid thread Tid that can be used for a newly created thread.
int findNextTid(){
	int i;
	for(i = 1; i < ULT_MAX_THREADS; i++){
		if(!validTid(i))
			return i;
	}
	return 1024;
}

// param toTest is the Tid being checked for validity.
// returns 1 if toTest is a valid thread in the runningQueue.
// 0 otherwise.
int validTid(Tid toTest){
	int i;
	for(i=0; i<readySize; i++)
	{
		if(readyQueue[i]->threadID == toTest)
			return 1;
	}
	return 0;
}

// param tid is the Tid of the thread to be destroyed.
// returns the Tid of the thread that was destroyed.
Tid ULT_DestroyThread(Tid tid)
{  
	if(firstTime)
		ULT_Initialize();

	//if there are no more thread return ult none
	if(tid == ULT_ANY && readySize ==0)
		return ULT_NONE;

	if(tid == ULT_SELF && readySize == 0)
		return ULT_NONE;

	//If deleting self, switch to another thread
	//current thread isn't on the queue so it doesn't
	//need to be removed
	if(tid == ULT_SELF){
		ThrdCtlBlk *nextThread;
		nextThread = dequeue();
		free(runningThread->context);
		runningThread = nextThread;
		setcontext(runningThread->context);
	}

	//If deleting any, delete the first on the queue
	if(tid == ULT_ANY){
		ThrdCtlBlk *toDelete = dequeue();
		Tid toReturn = toDelete->threadID;
		free(toDelete->context);
		free(toDelete);
		return toReturn;
	}

	if(!validTid(tid))
		return ULT_INVALID;

	//If there are no other threads, return ult none
	if(readySize == 0)
		return ULT_NONE;

	ThrdCtlBlk *toDelete = removeFromReady(tid);
	Tid toReturn = toDelete->threadID;
	free(toDelete->context);
	free(toDelete);
	return toReturn;
}





