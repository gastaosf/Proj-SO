#include "tecnicofs-client-api.h"
#include "errno.h"
#define socketName "/tmp/clientSocket"

int sockfd;
socklen_t servlen, clilen;
struct sockaddr_un servAddr, clientAddr;

int tfsCreate(char *filename, char nodeType)
{
  int in_buffer;
  char out_buffer[MAX_INPUT_SIZE];
  sprintf(out_buffer, "c %s %c\n", filename, nodeType);
  if (sendto(sockfd, out_buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&servAddr, servlen) < 0)
  {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  }

  if (recvfrom(sockfd, &in_buffer, sizeof(in_buffer), 0, 0, 0) <= 0)
  {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  return in_buffer;
}

int tfsDelete(char *path)
{
  int in_buffer;
  char out_buffer[MAX_INPUT_SIZE];
  sprintf(out_buffer, "d %s\n", path);
  if (sendto(sockfd, out_buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&servAddr, servlen) < 0)
  {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  }

  if (recvfrom(sockfd, &in_buffer, sizeof(in_buffer), 0, 0, 0) <= 0)
  {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  return in_buffer;
}

int tfsMove(char *from, char *to)
{
  int in_buffer;
  char out_buffer[MAX_INPUT_SIZE];
  sprintf(out_buffer, "m %s %s\n", from, to);
  if (sendto(sockfd, out_buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&servAddr, servlen) < 0)
  {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  }

  if (recvfrom(sockfd, &in_buffer, sizeof(in_buffer), 0, 0, 0) <= 0)
  {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  return in_buffer;
}

int tfsLookup(char *path)
{
  int in_buffer;
  char out_buffer[MAX_INPUT_SIZE];
  sprintf(out_buffer, "l %s\n", path);
  if (sendto(sockfd, out_buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&servAddr, servlen) < 0)
  {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  }

  if (recvfrom(sockfd, &in_buffer, sizeof(in_buffer), 0, 0, 0) <= 0)
  {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  return in_buffer;
}

int tfsPrint(char *path)
{
  int in_buffer;
  char out_buffer[MAX_INPUT_SIZE];
  sprintf(out_buffer, "p %s\n", path);
  if (sendto(sockfd, out_buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&servAddr, servlen) < 0)
  {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  }

  if (recvfrom(sockfd, &in_buffer, sizeof(in_buffer), 0, 0, 0) <= 0)
  {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }

  return in_buffer;
}

int tfsMount(char *sockPath)
{

  bzero((char *)(&servAddr), sizeof(struct sockaddr_un));
  (&servAddr)->sun_family = AF_UNIX;
  strcpy((&servAddr)->sun_path, sockPath);

  servlen = SUN_LEN(&servAddr);

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
  {
    perror("client: can't open socket");
    exit(EXIT_FAILURE);
  }
  unlink(socketName);

  bzero((char *)(&clientAddr), sizeof(struct sockaddr_un));
  (&clientAddr)->sun_family = AF_UNIX;
  strcpy((&clientAddr)->sun_path, socketName);

  clilen = SUN_LEN(&clientAddr);

  if (bind(sockfd, (struct sockaddr *)&clientAddr, clilen))
  {
    perror("client: bind error");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}

int tfsUnmount()
{
  if (close(sockfd))
  {
    perror("client: can't close socket");
    exit(EXIT_FAILURE);
  }
  if (unlink(socketName))
  {
    perror("client: can't unlink socket path on unmount");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}