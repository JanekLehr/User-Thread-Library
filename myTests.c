/*
/ Authors: Janek Lehr, Nick Blazier
/ February 17th, 2012
/ Tests basic functionality of ULT.c
*/

#include <assert.h>
#include <stdio.h>
#include "interrupt.h"
#include "basicThreadTests.h"
#include "ULT.h"
#include <unistd.h>
#include "interrupt.h"
#include "ULT.h"
#include <string.h>

void doALittleDance(char* arg);
void funkyTown(char* arg);

int main(int argc, char **argv)
{
	printf("Starting our tests....\n");
	
	Tid IAmNotValid = 5;
	
	Tid linen;
	Tid cotton;
	Tid silk;
	// Get it? Because they are threads...
	
	printf("Testing destruction of a nonexistent thread.\n");
	IAmNotValid = ULT_DestroyThread(IAmNotValid);
	assert(IAmNotValid == ULT_INVALID);
	
	printf("Testing creation of a thread and yielding to it.\n");
	linen = ULT_CreateThread((void (*)(void *))doALittleDance, "Make a little love\n");
	cotton = ULT_Yield(linen);
	assert(cotton == linen);
	
	printf("Testing destruction of a thread.\n");
	Tid ret = ULT_DestroyThread(linen);
	assert(ret == linen);
	
	printf("Testing creation of a thread with a more complex root function, and then yielding to it.\n");
	silk = ULT_CreateThread((void (*)(void *))funkyTown, "In the funkyTown method and the arg was passed just fine. Woohoo.\n");
	Tid cloth = ULT_Yield(silk);
	assert(cloth == silk);
	
	printf("Testing destruction of a thread that has already terminated naturally.\n");
	Tid deleteAttempt = ULT_DestroyThread(silk);
	assert(deleteAttempt == ULT_INVALID);
	
	printf("You have passed all your tests! Nice dude!\n");

	return 1;
}

// Tests creation, yielding, destruction, and yielding to a nonexistent thread
// arg is the string to print within the method.
void funkyTown(char* arg)
{
	printf("%s",arg);
	Tid weave = ULT_CreateThread((void (*)(void *))doALittleDance, "Make a little love\n");
	Tid ret = ULT_Yield(weave);
	assert(ret == weave);
	Tid ret2 = ULT_DestroyThread(weave);
	assert(ret2 == weave);
	Tid ret3 = ULT_Yield(ret2);
	assert(ret3 = ULT_INVALID);
}
	
// Tests that arg was passed correctly and yields to another thread on the readyQueue.
// arg is the string to compare.
void doALittleDance(char* arg)
{
	assert(strcmp(arg, "Make a little love\n") == 0);
	while(1)
	{
		ULT_Yield(ULT_ANY);
	}
}