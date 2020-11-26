#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include "operations.h"

#define MAX_INPUT_SIZE 100

int numberThreads;
pthread_t *tid;
pthread_rwlock_t lockFS = PTHREAD_RWLOCK_INITIALIZER;

void readlockFS()
{
    if (!pthread_rwlock_rdlock(&lockFS))
    {
        fprintf(stderr, "Error: while acquiring read lock for FS\n");
        exit(EXIT_FAILURE);
    }
}

void writelockFS()
{
    if (!pthread_rwlock_wrlock(&lockFS))
    {
        fprintf(stderr, "Error: while acquiring write lock for FS\n");
        exit(EXIT_FAILURE);
    }
}

void unlockFS()
{
    if (!pthread_rwlock_unlock(&lockFS))
    {
        fprintf(stderr, "Error: while releasing lock for FS\n");
        exit(EXIT_FAILURE);
    }
}

void errorParse()
{
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void createTaskPool(int numThreads, void *apply)
{
    tid = malloc(sizeof(pthread_t) * numberThreads);
    for (int i = 0; i < numberThreads; i++)
    {
        if (pthread_create(&tid[i], NULL, apply, NULL) != 0)
        {
            fprintf(stderr, "Error creating thread.\n");
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

void applyCommands(char *command)
{

    while (1)
    {
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
                writelockFS();
                printf("Create file: %s\n", name);
                create(name, T_FILE);
                unlockFS();
                break;
            case 'd':
                writelockFS();
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
            readlockFS();
            searchResult = lookup(name);
            if (searchResult >= 0)
                printf("Search: %s found\n", name);
            else
                printf("Search: %s not found\n", name);
            unlockFS();
            break;
        case 'd':
            writelockFS();
            printf("Delete: %s\n", name);
            delete (name);
            unlockFS();
            break;
        // case 'm':
        //     writelockFS();
        //     printf("Move: %s to %s\n",destination,source);
        //     delete (name);
        //     unlockFS();
        //     break;
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

    numberThreads = atoi(argv[1]);

    init_fs();
    createTaskPool(numberThreads, &applyCommands);
    joinTasks(numberThreads);
    destroy_fs();
    free(tid);
    exit(EXIT_SUCCESS);
}
