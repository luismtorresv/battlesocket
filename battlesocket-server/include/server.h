#ifndef SERVER_H
#define SERVER_H

#include "game.h"
#include "logger.h"
#include "protocol.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUMBER_OF_ROOMS 1024
#define SERVER_PORT 8080
#define MAX_CLIENTS 2

extern pthread_mutex_t room_mutex;

typedef struct Room Room;
typedef struct Client Client;
typedef struct ThreadInfo ThreadInfo;

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
  bool disconnected; // Flag to avoid repetition while handling disconnections.
  pthread_t thread;
};

struct ThreadInfo
{
  pthread_mutex_t mutex;
  Client client;
  Room *rooms;
};

void run_server ();

Board *get_board (Game *game, Player player);
Board *get_current_board (Game *game);
Board *get_opposing_board (Game *game);
int get_ship_index_at (Board *board, int row, int col);
void get_ship_data (Board *board, char *buffer, size_t buffer_size);

bool is_room_available (Room *room);
int get_current_socket_fd (Room *room);
Client *get_client (Room *room, Player player);
Client *get_current_client (Room *room);

void handle_message (Room *room, Client *client, char *message);
void send_start_game (Room *room, Player player);
void handle_disconnect (Room *room);
void *handle_game (void *arg);
void *handle_client (void *arg);

void send_to_client (Client *client, const char *message);
void broadcast (const char *message, Room *room);

#endif
