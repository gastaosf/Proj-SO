#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <pthread.h>

#include "synch.h"

/* synch strategy to be used to lock the fileSystem */
char *synchStrategy;

/* Initialiazes the synch mechanism .*/
void synchInit(char *synchStrategyInit, int numThreads)
{
    synchStrategy = strdup(synchStrategyInit);
    pthread_mutex_init(&lock_job_queue, NULL);
    if (!strcmp(synchStrategy, "mutex"))
    {
        pthread_mutex_init(&(lock_FS.mutex), NULL);
    }
    else if (!strcmp(synchStrategyInit, "rwlock"))
    {
        pthread_rwlock_init(&(lock_FS.rwlock), NULL);
    }
    else if (!strcmp(synchStrategy, "nosync"))
    {
        //no synch only works if there is only 1 thread
        if (numThreads != 1)
        {
            fprintf(stderr,"Error! nosync can only use 1 thread.");
            exit(1);
        }
    }
    else
    {
        fprintf(stderr,"Error! Unacceptable strategy. (check spelling)");
        exit(1);
    }
}


/* Lock FileSystem's internal structure */
void lockFS()
{
    if (!strcmp(synchStrategy, "mutex"))
    {
        pthread_mutex_lock(&(lock_FS.mutex));
    }
    else if (!strcmp(synchStrategy, "rwlock"))
    {
        pthread_rwlock_wrlock(&(lock_FS.rwlock));
    }
    else
    {
        ////nosync so does nothing
    }
}

/* Lock FileSystem's internal structure  to writing */
void lockFSReadOnly()
{
    if (!strcmp(synchStrategy, "mutex"))
    {
        pthread_mutex_lock(&(lock_FS.mutex));
    }
    else if (!strcmp(synchStrategy, "rwlock"))
    {
        pthread_rwlock_rdlock(&(lock_FS.rwlock));
    }
    else
    {
        //nosync so does nothing
    }
}

/* Unlock FileSystem's internal structure */
void unlockFS()
{
    if (!strcmp(synchStrategy, "mutex"))
    {
        pthread_mutex_unlock(&(lock_FS.mutex));
    }
    else if (!strcmp(synchStrategy, "rwlock"))
    {
        pthread_rwlock_unlock(&(lock_FS.rwlock));
    }
    else
    {
        //nosync so does nothing
    }
}


/* Lock acesss to the job queue */
void lockCommandVector()
{
    pthread_mutex_lock(&(lock_job_queue));
}

/* Unlock acesss to the job queue  */
void unlockCommandVector()
{
    pthread_mutex_unlock(&(lock_job_queue));
}

/* Terminate the synching mechanism */
void synchTerminate(char *synchStrategy)
{
    pthread_mutex_destroy(&lock_job_queue);
    if (!strcmp(synchStrategy, "mutex"))
    {
        pthread_mutex_destroy(&(lock_FS.mutex));
    }
    else if (!strcmp(synchStrategy, "rwlock"))
    {
        pthread_rwlock_destroy(&(lock_FS.rwlock));
    }
}
