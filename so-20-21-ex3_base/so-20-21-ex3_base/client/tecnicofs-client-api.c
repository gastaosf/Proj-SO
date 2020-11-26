#include "tecnicofs-client-api.h"

int sockfd;
socklen_t servlen, clilen;
struct sockaddr_un serv_addr, client_addr;
char* clientSockPath;
char buffer[MAX_INPUT_SIZE];

int setSockAddrUn(char *path, struct sockaddr_un *addr)
{
  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}

int tfsCreate(char *filename, char nodeType)
{
  sprintf(buffer, "c %s %c\n", filename, nodeType);
  if (sendto(sockfd, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&serv_addr, servlen) > 0)
  {
    perror("client: sendto error");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int tfsDelete(char *path)
{
  sprintf(buffer, "d %s\n", path);
  if (sendto(sockfd, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&serv_addr, servlen) > 0)
  {
    perror("client: sendto error");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int tfsMove(char *from, char *to)
{
  sprintf(buffer, "m %s %s\n", from, to);
  if (sendto(sockfd, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&serv_addr, servlen) > 0)
  {
    perror("client: sendto error");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int tfsLookup(char *path)
{
  sprintf(buffer, "l %s\n", path);
  if (sendto(sockfd, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&serv_addr, servlen) > 0)
  {
    perror("client: sendto error");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int tfsMount(char *sockPath)
{

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
  {
    perror("client: can't open socket");
    exit(EXIT_FAILURE);
  }

  unlink(sockPath);
  clilen = setSockAddrUn(sockPath, &client_addr);
  if (bind(sockfd, (struct sockaddr *)&client_addr, clilen) < 0)
  {
    perror("client: bind error");
    exit(EXIT_FAILURE);
  }

  strcpy(clientSockPath, sockPath);

  return EXIT_SUCCESS;
}

int tfsUnmount()
{
  close(sockfd);
  unlink(clientSockPath);

  return EXIT_SUCCESS;
}