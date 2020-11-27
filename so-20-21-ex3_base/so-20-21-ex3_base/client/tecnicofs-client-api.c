#include "tecnicofs-client-api.h"
#define socketName "/tmp/clientSocket"

int sockfd;
socklen_t servlen, clilen;
struct sockaddr_un serv_addr, client_addr;
char *clientSockPath = socketName;
char *serverSockPath = "";
char buffer[MAX_INPUT_SIZE];

int tfsCreate(char *filename, char nodeType)
{
  int in_buffer;
  char out_buffer[MAX_INPUT_SIZE];
  sprintf(out_buffer, "c %s %c\n", filename, nodeType);
  if (sendto(sockfd, out_buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&serv_addr, servlen) < 0)
  {
    perror("client: sendto error\n");
    return EXIT_FAILURE;
  }

  if (recvfrom(sockfd, &in_buffer, sizeof(buffer), 0, 0, 0) <= 0)
  {
    perror("client: recvfrom error\n");
    return EXIT_FAILURE;
  }

  return in_buffer;
}

int tfsDelete(char *path)
{
  int in_buffer;
  char out_buffer[MAX_INPUT_SIZE];
  sprintf(out_buffer, "d %s\n", path);
  if (sendto(sockfd, out_buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&serv_addr, servlen) < 0)
  {
    perror("client: sendto error\n");
    return EXIT_FAILURE;
  }

  if (recvfrom(sockfd, &in_buffer, sizeof(buffer), 0, 0, 0) <= 0)
  {
    perror("client: recvfrom error\n");
    return EXIT_FAILURE;
  }

  return in_buffer;
}

int tfsMove(char *from, char *to)
{
  int in_buffer;
  char out_buffer[MAX_INPUT_SIZE];
  sprintf(out_buffer, "m %s %s\n", from, to);
  if (sendto(sockfd, out_buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&serv_addr, servlen) < 0)
  {
    perror("client: sendto error\n");
    return EXIT_FAILURE;
  }

  if (recvfrom(sockfd, &in_buffer, sizeof(buffer), 0, 0, 0) <= 0)
  {
    perror("client: recvfrom error\n");
    return EXIT_FAILURE;
  }

  return in_buffer;
}

int tfsLookup(char *path)
{
  int in_buffer;
  char out_buffer[MAX_INPUT_SIZE];
  sprintf(out_buffer, "l %s\n", path);
  if (sendto(sockfd, out_buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *)&serv_addr, servlen) < 0)
  {
    perror("client: sendto error\n");
    return EXIT_FAILURE;
  }

  if (recvfrom(sockfd, &in_buffer, sizeof(buffer), 0, 0, 0) <= 0)
  {
    perror("client: recvfrom error\n");
    return EXIT_FAILURE;
  }

  return in_buffer;
}

int tfsMount(char *sockPath)
{
  bzero((char *)(&serv_addr), sizeof(struct sockaddr_un));
  (&serv_addr)->sun_family = AF_UNIX;
  strcpy((&serv_addr)->sun_path, sockPath);

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
  {
    perror("client: can't open socket\n");
    exit(EXIT_FAILURE);
  }

  unlink(socketName);

  clilen = SUN_LEN(&serv_addr);

  bzero((char *)(&client_addr), sizeof(struct sockaddr_un));
  (&client_addr)->sun_family = AF_UNIX;
  strcpy((&client_addr)->sun_path, clientSockPath);

  if (bind(sockfd, (struct sockaddr *)&client_addr, clilen) < 0)
  {
    perror("client: bind error\n");
    exit(EXIT_FAILURE);
  }
  servlen = SUN_LEN(&serv_addr);

  serverSockPath = sockPath;

  return EXIT_SUCCESS;
}

int tfsUnmount()
{
  if (close(sockfd) != 0)
  {
    perror("client: can't close socket\n");
    exit(EXIT_FAILURE);
  }
  if (unlink(clientSockPath) != 0)
  {
    perror("client: can't close socket\n");
    exit(EXIT_FAILURE);
  };

  return EXIT_SUCCESS;
}