#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include "operations.h"

#define MAX_INPUT_SIZE 100

int numberThreads;
pthread_t *tid;
pthread_rwlock_t lockFS = PTHREAD_RWLOCK_INITIALIZER;

int sockfd;
struct sockaddr_un serverAddr;
socklen_t addrlen;
char *serverName = "";

int setSockAddrUn(char *path, struct sockaddr_un *addr)
{
    bzero((char *)addr, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, path);

    return SUN_LEN(addr);
}

void readlockFS()
{
    if (pthread_rwlock_rdlock(&lockFS))
    {
        perror("Error: while acquiring read lock for FS");
        exit(EXIT_FAILURE);
    }
}

void writelockFS()
{
    if (pthread_rwlock_wrlock(&lockFS))
    {
        perror("Error: while acquiring write lock for FS");
        exit(EXIT_FAILURE);
    }
}

void unlockFS()
{
    if (pthread_rwlock_unlock(&lockFS))
    {
        perror("Error: while releasing lock for FS");
        exit(EXIT_FAILURE);
    }
}

void errorParse()
{
    perror("Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void createTaskPool(int numThreads, void *apply)
{
    tid = malloc(sizeof(pthread_t) * numberThreads);
    for (int i = 0; i < numberThreads; i++)
    {
        if (pthread_create(&tid[i], NULL, apply, NULL) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }
}

void joinTasks(int numberThreads)
{
    for (int i = 0; i < numberThreads; i++)
    {
        if (pthread_join(tid[i], NULL))
        {
            perror("joinTasks: pthread_join error");
            exit(EXIT_FAILURE);
        }
    }
}

void initServer()
{
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        perror("initServer: can't open socket");
        exit(EXIT_FAILURE);
    }
    if (unlink(serverName))
    {
        if(errno != ENOENT){
            perror("initServer: can't unlink given path");
            exit(EXIT_FAILURE);
        }
        
    }
    addrlen = setSockAddrUn(serverName, &serverAddr);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, addrlen) < 0)
    {
        perror("initServer: bind error");
        exit(EXIT_FAILURE);
    }
}

char *receiveCommand(struct sockaddr_un *client_addr)
{

    char buffer[MAX_INPUT_SIZE];
    char *command = malloc(MAX_INPUT_SIZE * sizeof(char));
    int c;

    addrlen = sizeof(struct sockaddr_un);
    c = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                 (struct sockaddr *)client_addr, &addrlen);

    if (c <= 0)
    {
        perror("receiveCommand : recvfrom error");
        exit(EXIT_FAILURE);
    }

    buffer[c] = '\0';
    strncpy(command, buffer, sizeof(buffer) - 1);
    return command;
}

void sendResponse(int response, struct sockaddr_un *client_addr)
{
    if (sendto(sockfd, &response, sizeof(response) + 1, 0, (struct sockaddr *)client_addr, addrlen) < 0)
    {
        perror("sendResponse: sendto error");
        exit(EXIT_FAILURE);
    }
}

void applyCommands()
{

    while (1)
    {
        char op;
        char arg1[MAX_INPUT_SIZE], arg2[MAX_INPUT_SIZE];
        int result;

        struct sockaddr_un client_addr;
        char *command = receiveCommand(&client_addr);

        int numTokens = sscanf(command, "%c %s %s", &op, arg1, arg2);
        if (numTokens < 2)
        {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        switch (op)
        {
        case 'c':
            switch (arg2[0])
            {
            case 'f':
                writelockFS();

                printf("Create file: %s\n", arg1);
                result = create(arg1, T_FILE);

                unlockFS();
                break;
            case 'd':
                writelockFS();

                printf("Create directory: %s\n", arg1);
                result = create(arg1, T_DIRECTORY);

                unlockFS();
                break;
            default:
                fprintf(stderr, "Error: invalid node type\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'l':
            readlockFS();

            result = lookup(arg1);
            if (result >= 0)
                printf("Search: %s found\n", arg1);
            else
                printf("Search: %s not found\n", arg1);

            unlockFS();
            break;
        case 'd':
            writelockFS();

            printf("Delete: %s\n", arg1);
            result = delete (arg1);

            unlockFS();
            break;
        case 'm':
            writelockFS();

            printf("Move: %s to %s\n", arg1, arg2);
            result = move(arg1, arg2);

            unlockFS();
            break;
        case 'p':
            writelockFS();

            printf("Print tree to: %s\n", arg1);
            result = print_tecnicofs_tree(arg1);

            unlockFS();
            break;
        default:
        { /* error */
            fprintf(stderr, "Error: command to apply\n");
            exit(EXIT_FAILURE);
        }
        }

        sendResponse(result, &client_addr);
        free(command);
    }
}

static void parseArgs(long argc, char *const argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Invalid format:\n");
        printf("Usage: %s numberThreads serverName\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    numberThreads = atoi(argv[1]);
    serverName = argv[2];

    if (numberThreads <= 0)
    {
        fprintf(stderr, "Error: numberThreads must be a positive number\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{

    parseArgs(argc, argv);

    init_fs();
    initServer();
    createTaskPool(numberThreads, &applyCommands);
    joinTasks(numberThreads);
    destroy_fs();
    free(tid);
    exit(EXIT_SUCCESS);
}
