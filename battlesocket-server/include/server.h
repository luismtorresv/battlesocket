#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

#define SERVER_PORT 8080
#define MAX_CLIENTS 2

typedef struct
{
  int sockfd;
  struct sockaddr_in addr;
  char letter;
} Client;

int check_connection();
void init_server ();
void run_server ();
void cleanup_server ();

#endif
