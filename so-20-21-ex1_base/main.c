/*
90454 - Gastão Faria
9 - Manuel Brito
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include "fs/operations.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

pthread_rwlock_t rwlock_FS;
int numberThreads = 0;
char * synchStrategy = "";
pthread_t tid[12];
pthread_mutex_t lock_job_queue = PTHREAD_MUTEX_INITIALIZER;

union lock_FS
{
    pthread_mutex_t mutex;
    pthread_rwlock_t rwlock;
} lock_FS;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

int insertCommand(char * data)
{
    if (numberCommands != MAX_COMMANDS)
    {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char * removeCommand()
{
    if (numberCommands > 0)
    {
        numberCommands--;
        return inputCommands[headQueue++];
    }
    return NULL;
}

void errorParse()
{
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void synchInit(char * synchStrategy)
{
    if (!strcmp(synchStrategy, "mutex"))
    {
        pthread_mutex_init(&(lock_FS.mutex), NULL);
    }
    else if (!strcmp(synchStrategy, "rwlock"))
    {
        pthread_rwlock_init(&(lock_FS.rwlock), NULL);
    }
}

void synchTerminate(char * synchStrategy)
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

void createTaskPool(int numThreads, void * apply)
{
    for (int i = 0; i < numberThreads; i++)
    {
        if (pthread_create(&tid[i], NULL, apply, NULL) != 0)
        {
            printf("Error creating thread.\n");
            exit(EXIT_FAILURE);
        }
    }
}

void joinTasks(int numberThreads)
{
    for (int i = 0; i < numberThreads; i++)
    {
        pthread_join(tid[i], NULL);
    }
}

void lockFS()
{
    if (!strcmp(synchStrategy, "mutex"))
    {
        pthread_mutex_lock(&(lock_FS.mutex));
    }
    else
    {
        pthread_rwlock_wrlock(&(lock_FS.rwlock));
    }
}

void lockFSReadOnly()
{
    if (!strcmp(synchStrategy, "mutex"))
    {
        pthread_mutex_lock(&(lock_FS.mutex));
    }
    else
    {
        pthread_rwlock_rdlock(&(lock_FS.rwlock));
    }
}

void unlockFS()
{
    if (!strcmp(synchStrategy, "mutex"))
    {
        pthread_mutex_unlock(&(lock_FS.mutex));
    }
    else
    {
        pthread_rwlock_unlock(&(lock_FS.rwlock));
    }
}

void processInput(FILE * fp)
{
    char line[MAX_INPUT_SIZE];

    /* break loop with ^Z or ^D */
    while (fgets(line, sizeof(line) / sizeof(char), fp))
    {
        char token, type;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s %c", &token, name, &type);

        /* perform minimal validation */
        if (numTokens < 1)
        {
            continue;
        }
        switch (token)
        {
        case 'c':
            if (numTokens != 3)
                errorParse();
            if (insertCommand(line))
                break;
            return;

        case 'l':
            if (numTokens != 2)
                errorParse();
            if (insertCommand(line))
                break;
            return;

        case 'd':
            if (numTokens != 2)
                errorParse();
            if (insertCommand(line))
                break;
            return;

        case '#':
            break;

        default:
        { /* error */
            errorParse();
        }
        }
    }
}

void applyCommands()
{

    // while (numberCommands > 0)
    while (1)
    {
        pthread_mutex_lock(&lock_job_queue);
        const char * command = removeCommand();

        if (command == NULL)
        {
            pthread_mutex_unlock(&lock_job_queue);
            break;
        }

        char token, type;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        if (numTokens < 2)
        {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;

        pthread_mutex_unlock(&lock_job_queue);

        switch (token)
        {
        case 'c':
            switch (type)
            {
            case 'f':
                printf("Create file: %s\n", name);
                lockFS();
                create(name, T_FILE);
                unlockFS();
                break;
            case 'd':
                printf("Create directory: %s\n", name);
                lockFS();
                create(name, T_DIRECTORY);
                unlockFS();
                break;
            default:
                fprintf(stderr, "Error: invalid node type\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'l':
            lockFSReadOnly();
            searchResult = lookup(name);

            if (searchResult >= 0)
                printf("Search: %s found\n", name);
            else
                printf("Search: %s not found\n", name);
            unlockFS();
            break;
        case 'd':
            lockFS();
            printf("Delete: %s\n", name);
            delete (name);
            unlockFS();
            break;
        default:
        { /* error */
            fprintf(stderr, "Error: command to apply\n");
            exit(EXIT_FAILURE);
        }
        }
    }
}

/* Ensures that number of arguments given is correct. */
void argNumChecker(int argc)
{
    if(argc != 5){
      perror("Error! Wrong number of arguments given.");
      exit(1);
    }
}

/* Ensures that input file exists and there are no problems. */
FILE * inputFileHandler(char * file_name)
{
    FILE * fp;
    fp = fopen(file_name, "r");

    if (fp == NULL){
      perror("Error! No input file with such name.");
      exit(1);
    }
    return fp;
}

/* Ensures that output file has no problems. */
FILE * outputFileHandler(char * file_name)
{
    FILE * fp;
    fp = fopen(file_name, "w");

    if (fp == NULL){
      perror("Error! Output file was not created.");
      exit(1);
    }
    return fp;
}

/* Ensures number of threads is possible. */
int numThreadsHandler(char * num_threads)
{
    int threads = atoi(num_threads);

    if(threads <= 0){
        perror("Error! Number of threads is either negative or zero.");
        exit(1);
        return -1;
    }

    return threads;
}

/* Ensures synch strategy is allowed. */
void checkSynchStrategy(char * synchStrategy)
{
    if(!(strcmp(synchStrategy, "nosync")==0||strcmp(synchStrategy,"mutex")==0||
       strcmp(synchStrategy,"rwlock")==0)){
           perror("Error! Unacceptable strategy. (check spelling)");
           exit(1);
       }
}

void checkNumThreads(char * num_threads, char * synchStrategy)
{
    int threads = atoi(num_threads);

    if(threads != 1 && strcmp(synchStrategy, "nosync")==0){
        perror("Error! nosync only uses 1 thread.");
        exit(1);
    }

}

int main(int argc, char * argv[])
{

    struct timeval start, end;
    double time;

    argNumChecker(argc);

    FILE * fp = inputFileHandler(argv[1]);
    FILE * fp2 = outputFileHandler(argv[2]);

    numberThreads = numThreadsHandler(argv[3]);

    checkSynchStrategy(argv[4]);

    checkNumThreads(argv[3], argv[4]);

    synchStrategy = strdup(argv[4]);

    synchInit(synchStrategy);

    /* init filesystem */
    init_fs();

    /* process input and print tree */
    processInput(fp);

    /* Create task pool */
    gettimeofday(&start, NULL);

    createTaskPool(numberThreads, &applyCommands);
    joinTasks(numberThreads);

    synchTerminate(synchStrategy);

    /* get run time*/
    gettimeofday(&end, NULL);
    time = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0;
    print_tecnicofs_tree(fp2);
    printf("TecnicoFS completed in %.4lf seconds.\n", time);

    fclose(fp);
    fclose(fp2);
    /* release allocated memory */
    destroy_fs();
    exit(EXIT_SUCCESS);
}
