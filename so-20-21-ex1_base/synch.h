#ifndef SYNCH_H
#define SYNCH_H

<<<<<<< HEAD
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t lock_job_queue;

=======
pthread_mutex_t lock_job_queue;
>>>>>>> main
union lock_FS
{
    pthread_mutex_t mutex;
    pthread_rwlock_t rwlock;
} lock_FS;

<<<<<<< HEAD
char *synchStrategy;

/* Initialize the synching mechanism. */
void synchInit(char *synchStrategy, int numThreads);

/* Lock FileSystem's internal structure. */
void lockFS();

/* Lock FileSystem's internal structure to writing. */
void lockFSReadOnly();

/* Unlock FileSystem's internal structure. */
void unlockFS();

/* Lock acesss to the job queue. */
void lockCommandVector();

/* Unlock acesss to the job queue. */
void unlockCommandVector();

/* Terminate the synching mechanism. */
void synchTerminate(char *synchStrategy);
=======
char* synchStrategy;

/* Initialize the synching mechanism */
//void synchInit(int numThreads);



/* Lock acesss to the job queue */
void lockCommandVector();

/* Unlock acesss to the job queue  */
void unlockCommandVector();

/* Terminate the synching mechanism */
//void synchTerminate(char* synchStrategy);
>>>>>>> main

#endif /* SYNCH_H */
