/*
90454 - Gastão Faria
95623 - Manuel Brito
Grupo 98
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
#include "synch.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

<<<<<<< HEAD
pthread_rwlock_t rwlock_FS;
int numberThreads = 0;
char *synchStrategy = "";
pthread_t tid[12];
pthread_mutex_t lock_job_queue = PTHREAD_MUTEX_INITIALIZER;

union lock_FS
{
    pthread_mutex_t mutex;
    pthread_rwlock_t rwlock;
} lock_FS;
=======
int numberThreads;
char *synchStrategy = "";
pthread_t *tid;
>>>>>>> main

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

int insertCommand(char *data)
{
    if (numberCommands != MAX_COMMANDS)
    {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char *removeCommand()
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

<<<<<<< HEAD
void synchInit(char *synchStrategy)
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

=======
/* creates task pool with numThreads threads */
>>>>>>> main
void createTaskPool(int numThreads, void *apply)
{
    tid = malloc(sizeof(pthread_t) * numberThreads);
    for (int i = 0; i < numberThreads; i++)
    {
        if (pthread_create(&tid[i], NULL, apply, NULL) != 0)
        {
            fprintf(stderr,"Error creating thread.\n");
            exit(EXIT_FAILURE);
        }
    }
}

/* join tasks */
void joinTasks(int numberThreads)
{
    for (int i = 0; i < numberThreads; i++)
    {
        pthread_join(tid[i], NULL);
    }
}

<<<<<<< HEAD
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

=======
/* Ensures that number of arguments given is correct. */
void argNumChecker(int argc){
    if(argc != 5){
      fprintf(stderr, "Wrong number of arguments given.");
      exit(1);
    }
}

/* Ensures that input file exists and there are no problems. */
FILE * inputFileHandler(char * file_name){
    FILE *fp;
    fp = fopen(file_name, "r");

    if (fp == NULL){
      fprintf(stderr, "No input file with such name.");
      exit(1);
    }
    return fp;
}

/* Ensures that output file has no problems. */
FILE * outputFileHandler(char * file_name){
    FILE *fp;
    fp = fopen(file_name, "w");

    if (fp == NULL){
      fprintf(stderr, "Output file was not created.");
      exit(1);
    }
    return fp;
}

/* Ensures number of threads is possible. */
int numThreadsHandler(char * num_threads){
    int threads = atoi(num_threads);

    if(threads <= 0){
        fprintf(stderr, "Number of threads is either negative or zero.");
        exit(1);
        return -1;
    }

    return threads;
}


>>>>>>> main
void processInput(FILE *fp)
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

<<<<<<< HEAD
    /* while (numberCommands > 0) */
=======
>>>>>>> main
    while (1)
    {
        lockCommandVector();
        const char *command = removeCommand();
        unlockCommandVector();

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
<<<<<<< HEAD

        pthread_mutex_unlock(&lock_job_queue);

=======
>>>>>>> main
        switch (token)
        {
        case 'c':
            switch (type)
            {
            case 'f':
                lockFS();
                printf("Create file: %s\n", name);
                lockFS();
                create(name, T_FILE);
                unlockFS();
                break;
            case 'd':
                lockFS();
                printf("Create directory: %s\n", name);
                lockFS();
                create(name, T_DIRECTORY);
                unlockFS();
<<<<<<< HEAD
=======

>>>>>>> main
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
<<<<<<< HEAD
    }
}

/* Ensures that number of arguments given is correct. */
void argNumChecker(int argc)
{
    if (argc != 5)
    {
        perror("Error! Wrong number of arguments given.");
        exit(1);
    }
}

/* Ensures that input file exists and there are no problems. */
FILE *inputFileHandler(char *file_name)
{
    FILE *fp;
    fp = fopen(file_name, "r");

    if (fp == NULL)
    {
        perror("Error! No input file with such name.");
        exit(1);
    }
    return fp;
}

/* Ensures that output file has no problems. */
FILE *outputFileHandler(char *file_name)
{
    FILE *fp;
    fp = fopen(file_name, "w");

    if (fp == NULL)
    {
        perror("Error! Output file was not created.");
        exit(1);
    }
    return fp;
}

/* Ensures number of threads is possible. */
int numThreadsHandler(char *numberThreads)
{
    int threads = atoi(numberThreads);

    if (threads <= 0)
    {
        perror("Error! Number of threads is either negative or zero.");
        exit(1);
        return -1;
    }

    return threads;
}

/* Ensures synch strategy is allowed. */
void checkSynchStrategy(char *synchStrategy)
{
    if (!(strcmp(synchStrategy, "nosync") == 0 ||
          strcmp(synchStrategy, "mutex") == 0 ||
          strcmp(synchStrategy, "rwlock") == 0))
    {
        perror("Error! Unacceptable strategy. (check spelling)");
        exit(1);
    }
}

/* Ensures there is only 1 thread when using nosync. */
void checkNumThreads(char *numberThreads, char *synchStrategy)
{
    int threads = atoi(numberThreads);

    if (threads != 1 && strcmp(synchStrategy, "nosync") == 0)
    {
        perror("Error! nosync can only use 1 thread.");
        exit(1);
=======
>>>>>>> main
    }
}

int main(int argc, char *argv[])
{

    struct timeval start, end;
    double time;

    argNumChecker(argc);

    FILE *fp = inputFileHandler(argv[1]);
    FILE *fp2 = outputFileHandler(argv[2]);

    numberThreads = numThreadsHandler(argv[3]);
<<<<<<< HEAD

    checkSynchStrategy(argv[4]);

    checkNumThreads(argv[3], argv[4]);

    synchStrategy = strdup(argv[4]);
=======
    synchStrategy = argv[4];
    synchInit(synchStrategy, numberThreads);

>>>>>>> main

    synchInit(synchStrategy);

    /* Initiate filesystem. */
    init_fs();

    /* Process input and print tree. */
    processInput(fp);

    gettimeofday(&start, NULL);

<<<<<<< HEAD
    /* Create task pool. */
=======
>>>>>>> main
    createTaskPool(numberThreads, &applyCommands);
    joinTasks(numberThreads);

    synchTerminate(synchStrategy);

    /* Get run time. */
    gettimeofday(&end, NULL);
    time = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0;
    print_tecnicofs_tree(fp2);
    printf("TecnicoFS completed in %.4lf seconds.\n", time);

    fclose(fp);
    fclose(fp2);

<<<<<<< HEAD
    /* Eelease allocated memory. */
=======
    /* release allocated memory */
>>>>>>> main
    destroy_fs();
    free(tid);
    exit(EXIT_SUCCESS);
}
