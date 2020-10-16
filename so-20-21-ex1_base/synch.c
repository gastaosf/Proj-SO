#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "synch.h"

char * synchStrategy;

/* Initialize the synching mechanism. */
void synchInit(char * synchStrategyInit,int numThreads)
{
    synchStrategy = strdup(synchStrategyInit);
    pthread_mutex_init(&lock_job_queue,NULL);

    if (!strcmp(synchStrategy, "mutex"))
        pthread_mutex_init(&(lock_FS.mutex), NULL);

    else if (!strcmp(synchStrategy, "rwlock"))
        pthread_rwlock_init(&(lock_FS.rwlock), NULL);
}

/* Lock FileSystem's internal structure. */
void lockFS()
{
    if (!strcmp(synchStrategy, "mutex"))
        pthread_mutex_lock(&(lock_FS.mutex));
    else
        pthread_rwlock_rdlock(&(lock_FS.rwlock));
}

/* Lock FileSystem's internal structure to writing. */
void lockFSReadOnly()
{
    if (!strcmp(synchStrategy, "mutex"))
        pthread_mutex_lock(&(lock_FS.mutex));
    else
        pthread_rwlock_rdlock(&(lock_FS.rwlock));
}

/* Unlock FileSystem's internal structure. */
void unlockFS()
{
    if (!strcmp(synchStrategy, "mutex"))
        pthread_mutex_unlock(&(lock_FS.mutex));
    else
        pthread_rwlock_unlock(&(lock_FS.rwlock));
}

/* Lock acesss to the job queue */
void lockCommandVector()
{
    pthread_mutex_lock(&(lock_job_queue));
}

/* Unlock acesss to the job queue. */
void unlockCommandVector()
{
    pthread_mutex_unlock(&(lock_job_queue));
}

/* Terminate the synching mechanism. */
void synchTerminate(char * synchStrategy)
{
    pthread_mutex_destroy(&lock_job_queue);

    if (!strcmp(synchStrategy, "mutex"))
        pthread_mutex_destroy(&(lock_FS.mutex));

    else if (!strcmp(synchStrategy, "rwlock"))
        pthread_rwlock_destroy(&(lock_FS.rwlock));
}
