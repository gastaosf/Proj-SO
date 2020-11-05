#ifndef SYNCH_H
#define SYNCH_H

pthread_mutex_t lock_job_queue;
union lock_FS
{
    pthread_mutex_t mutex;
    pthread_rwlock_t rwlock;
} lock_FS;

char* synchStrategy;

/* Initialize the synching mechanism */
//void synchInit(int numThreads);



/* Lock acesss to the job queue */
void lockCommandVector();

/* Unlock acesss to the job queue  */
void unlockCommandVector();

/* Terminate the synching mechanism */
//void synchTerminate(char* synchStrategy);

#endif /* SYNCH_H */
