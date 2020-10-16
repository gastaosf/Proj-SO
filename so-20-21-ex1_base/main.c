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

int numberThreads;
char *synchStrategy = "";
pthread_t *tid;

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

/* creates task pool with numThreads threads */
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

    while (1)
    {
        lockCommandVector();
        const char *command = removeCommand();
        unlockCommandVector();

        if (command == NULL)
        {
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
        switch (token)
        {
        case 'c':
            switch (type)
            {
            case 'f':
                lockFS();
                printf("Create file: %s\n", name);
                create(name, T_FILE);
                unlockFS();
                break;
            case 'd':
                lockFS();
                printf("Create directory: %s\n", name);
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

int main(int argc, char *argv[])
{

    struct timeval start, end;
    double time;

    argNumChecker(argc);

    FILE *fp = inputFileHandler(argv[1]);
    FILE *fp2 = outputFileHandler(argv[2]);

    numberThreads = numThreadsHandler(argv[3]);
    synchStrategy = argv[4];
    synchInit(synchStrategy, numberThreads);


    /* init filesystem */
    init_fs();

    /* process input and print tree */
    processInput(fp);

    /* Create task pool */
    gettimeofday(&start, NULL);

    createTaskPool(numberThreads, &applyCommands);
    joinTasks(numberThreads);

    synchTerminate(synchStrategy);

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
