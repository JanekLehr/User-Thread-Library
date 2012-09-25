#ifndef _ULT_H_
#define _ULT_H_
#include <ucontext.h>

typedef int Tid;
#define ULT_MAX_THREADS 1024
#define ULT_MIN_STACK 32768


// The thread control block that will make up the elements of the readyQueue.
typedef struct ThrdCtlBlk{
  Tid threadID;
  ucontext_t *context;
} ThrdCtlBlk;


/*
 * Tids between 0 and ULT_MAX_THREADS-1 may
 * refer to specific threads and negative Tids
 * are used for error codes or control codes.
 * The first thread to run (before ULT_CreateThread
 * is called for the first time) must be Tid 0.
 */
static const Tid ULT_ANY = -1;
static const Tid ULT_SELF = -2;
static const Tid ULT_INVALID = -3;
static const Tid ULT_NONE = -4;
static const Tid ULT_NOMORE = -5;
static const Tid ULT_NOMEMORY = -6;
static const Tid ULT_FAILED = -7;
static const Tid ULT_ORIG = 0;

struct Node *head;

ThrdCtlBlk *runningThread;
ThrdCtlBlk *readyQueue[ULT_MAX_THREADS];
int readySize;


static inline int ULT_isOKRet(Tid ret){
  return (ret >= 0 ? 1 : 0);
}

Tid ULT_CreateThread(void (*fn)(void *), void *parg);
Tid ULT_Yield(Tid tid);
Tid ULT_DestroyThread(Tid tid);


 




#endif



