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
#define SYNCH_STRATEGY 7

int numberThreads = 0;
char *synchStrategy = "";
pthread_t tid[12];
pthread_mutex_t lock_job_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_FS = PTHREAD_MUTEX_INITIALIZER;

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

void createTaskPool(int numThreads, void *apply)
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

    // while (numberCommands > 0)
    while (1)
    {
        pthread_mutex_lock(&lock_job_queue);
        const char *command = removeCommand();

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

        pthread_mutex_lock(&lock_FS);
        pthread_mutex_unlock(&lock_job_queue);

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
                fprintf(stderr, "Error: invalid node type\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'l':
            searchResult = lookup(name);
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
        pthread_mutex_unlock(&lock_FS);
    }
}

int main(int argc, char *argv[])
{

    struct timeval start, end;
    double time;

    FILE *fp;
    fp = fopen(argv[1], "r");
    FILE *fp2;
    fp2 = fopen(argv[2], "w");
    numberThreads = atoi(argv[3]);
    synchStrategy = strdup(argv[4]);

    /* init filesystem */
    init_fs();

    /* process input and print tree */
    processInput(fp);

    /* Create task pool */
    gettimeofday(&start, NULL);
    createTaskPool(numberThreads, &applyCommands);
    joinTasks(numberThreads);

    pthread_mutex_destroy(&lock_job_queue);
    pthread_mutex_destroy(&lock_FS);

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
