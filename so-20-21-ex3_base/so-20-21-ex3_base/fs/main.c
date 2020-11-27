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

#include "operations.h"

#define MAX_INPUT_SIZE 100

int numberThreads;
pthread_t *tid;
pthread_rwlock_t lockFS = PTHREAD_RWLOCK_INITIALIZER;

int sockfd;
struct sockaddr_un server_addr;

socklen_t addrlen;
char *socket_path = "";

int setSockAddrUn(char *path, struct sockaddr_un *addr)
{

    if (addr == NULL)
        return 0;

    bzero((char *)addr, sizeof(struct sockaddr_un)); // why set to zero ??
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, path);

    return SUN_LEN(addr);
}

void readlockFS()
{
    if (pthread_rwlock_rdlock(&lockFS))
    {
        fprintf(stderr, "Error: while acquiring read lock for FS\n");
        exit(EXIT_FAILURE);
    }
}

void writelockFS()
{
    if (pthread_rwlock_wrlock(&lockFS))
    {
        fprintf(stderr, "Error: while acquiring write lock for FS\n");
        exit(EXIT_FAILURE);
    }
}

void unlockFS()
{
    if (pthread_rwlock_unlock(&lockFS))
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

void initServer(char *path)
{
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        perror("server: can't open socket");
        exit(EXIT_FAILURE);
    }
    unlink(path);
    socket_path = path;
    addrlen = setSockAddrUn(path, &server_addr);

    if (bind(sockfd, (struct sockaddr *)&server_addr, addrlen) < 0)
    {
        perror("server: bind error");
        exit(EXIT_FAILURE);
    }
}

char *receiveCommand(struct sockaddr_un *client_addr)
{
    while (1)
    {
        char buffer[MAX_INPUT_SIZE];
        char *command = malloc(MAX_INPUT_SIZE * sizeof(char));
        int c;

        addrlen = sizeof(struct sockaddr_un);
        c = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                     (struct sockaddr *)client_addr, &addrlen);

        if (c <= 0)
            continue;

        buffer[c] = '\0';
        strncpy(command, buffer, sizeof(buffer) - 1);
        return command;
    }
}

void sendResponse(int response, struct sockaddr_un *client_addr)
{
    sendto(sockfd, &response, sizeof(response) + 1, 0, (struct sockaddr *)client_addr, addrlen);
}

void applyCommands()
{

    while (1)
    {
        char token, type;
        char name[MAX_INPUT_SIZE];
        //writelockFS();
        struct sockaddr_un client_addr;
        char *command = receiveCommand(&client_addr);

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

                searchResult = create(name, T_FILE);
                sendResponse(searchResult, &client_addr);

                unlockFS();
                break;
            case 'd':
                writelockFS();
                printf("Create directory: %s\n", name);
                searchResult = create(name, T_DIRECTORY);
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
            searchResult = delete (name);
            unlockFS();
            break;
        // case 'm':
        //     writelockFS();
        //     printf("Move: %s to %s\n",source,destination);
        //     searchResult = move(source,destination);
        //     unlockFS();
        //     break;
        default:
        { /* error */
            fprintf(stderr, "Error: command to apply\n");
            exit(EXIT_FAILURE);
        }
        }
        // writelockFS();
        // unlockFS();
    }
}

int main(int argc, char *argv[])
{

    numberThreads = atoi(argv[1]);

    init_fs();
    initServer(argv[2]);
    createTaskPool(numberThreads, &applyCommands);
    joinTasks(numberThreads);
    destroy_fs();
    free(tid);
    exit(EXIT_SUCCESS);
}
