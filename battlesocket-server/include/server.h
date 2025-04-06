#ifndef SERVER_H
#define SERVER_H

#include "game.h"
#include <arpa/inet.h>

typedef struct Room Room;
typedef struct Client Client;

struct Client
{
  int sockfd;
  struct sockaddr_in addr;
  Room *room;
  Player player;
};

struct Room
{
  int id;
  Client client_a;
  Client client_b;
  Game game;
};

void run_server ();

#endif
