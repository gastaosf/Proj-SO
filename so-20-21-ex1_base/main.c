#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include "fs/operations.h"

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100
#define TRUE 1

int numberThreads;

pthread_t *tid;
pthread_mutex_t lockQueue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t canAddCommand;
pthread_cond_t canRemoveCommand;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int numberCommandsTotal = 0;

int headQueue = 0;
int reachedEOF = !TRUE;

/* Lock acesss to the job queue */
void lockCommandVector()
{
    if (pthread_mutex_lock(&(lockQueue)))
    {
        printf("Error while locking queue ...");
        exit(EXIT_FAILURE);
    }
}

/* Unlock acesss to the job queue  */
void unlockCommandVector()
{
    if (pthread_mutex_unlock(&(lockQueue)))
    {
        printf("Error while unlocking queue ...");
        exit(EXIT_FAILURE);
    }
}

int insertCommand(char *data)
{
    lockCommandVector();
    while (numberCommands == MAX_COMMANDS)
    {
        pthread_cond_wait(&canAddCommand, &lockQueue);
    }
    strcpy(inputCommands[numberCommandsTotal % MAX_COMMANDS], data);

    numberCommands++;
    numberCommandsTotal++;

    pthread_cond_signal(&canRemoveCommand);
    unlockCommandVector();

    return 1;
}

char *removeCommand()
{
    char *command = "";
    // lockCommandVector();
    while (numberCommands == 0)
    {
        // if(!reachedEOF)
        //     return "";
        pthread_cond_wait(&canRemoveCommand, &lockQueue);
    }
    numberCommands--;

    command = inputCommands[headQueue % MAX_COMMANDS];

    headQueue++;
    pthread_cond_signal(&canAddCommand);

    return command;
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
            fprintf(stderr, "Error while creating thread.\n");
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
void argNumChecker(int argc)
{
    if (argc != 4)
    {
        fprintf(stderr, "Wrong number of arguments given.%d given %d required\n", argc, 4);
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
        fprintf(stderr, "No input file with such name. %s\n", file_name);
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
        fprintf(stderr, "Output file was not opened. %s\n", file_name);
        exit(1);
    }
    return fp;
}

/* Ensures number of threads is possible. */
int numThreadsHandler(char *num_threads)
{
    int threads = atoi(num_threads);

    if (threads <= 0)
    {
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
    lockCommandVector();
    reachedEOF = TRUE;
    pthread_cond_signal(&canRemoveCommand);
    unlockCommandVector();
}

/* Reads input */
// void readInput(FILE *fp)
// {
//     while (1)
//     {
//         lockCommandVector();
//         if (reachedEOF)
//         {
//             unlockCommandVector();
//             break;
//         }
//         while (numberCommands == MAX_COMMANDS)
//         {
//             pthread_cond_wait(&canAddCommand, &lockQueue);
//         }
//         processInput(fp);
//         pthread_cond_signal(&canRemoveCommand);
//         unlockCommandVector();
//     }
// }

void applyCommands()
{
    while (TRUE)
    {
        lockCommandVector();
        if ((reachedEOF && !numberCommands))
        {
            unlockCommandVector();
            break;
        }
        const char *command = removeCommand();

        char token, type;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        unlockCommandVector();

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
                printf("Create file: %s\n", name);
                create(name, T_FILE);
                break;
            case 'd':
                printf("Create directory: %s\n", name);
                create(name, T_DIRECTORY);
                break;
            default:
                fprintf(stderr, "Error: invalid node type in %s\n",command);
                printf("token-> %c name->%s type%c/n", token, name, type);
                exit(EXIT_FAILURE);
            }
            break;
        case 'l':
            searchResult = lookup_aux(name);
            if (searchResult >= 0)
                printf("Search: %s found\n", name);
            else
                printf("Search: %s not found\n", name);
            break;
        case 'd':
            printf("Delete: %s\n", name);
            delete (name);
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
    pthread_cond_init(&canAddCommand, NULL);
    pthread_cond_init(&canRemoveCommand, NULL);

    argNumChecker(argc);

    FILE *fp = inputFileHandler(argv[1]);
    FILE *fp2 = outputFileHandler(argv[2]);

    numberThreads = numThreadsHandler(argv[3]);

    /* init filesystem */
    init_fs();

    /* Creates task pool */
    createTaskPool(numberThreads, &applyCommands);

    /* initial time*/
    gettimeofday(&start, NULL);

    /* process input */
    processInput(fp);

    joinTasks(numberThreads);

    /* final time */
    gettimeofday(&end, NULL);

    time = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0;
    print_tecnicofs_tree(fp2);
    printf("TecnicoFS completed in %.4lf seconds.\n", time);

    fclose(fp);
    fclose(fp2);

    for (int i = 0; i < MAX_COMMANDS; i++)
    {
        printf("%s", inputCommands[i]);
    }

    /* release allocated memory */
    destroy_fs();
    pthread_mutex_destroy(&lockQueue);
    pthread_cond_destroy(&canAddCommand);
    pthread_cond_destroy(&canRemoveCommand);
    free(tid);
    exit(EXIT_SUCCESS);
}
