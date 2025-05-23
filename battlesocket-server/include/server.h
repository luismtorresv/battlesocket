#ifndef SERVER_H
#define SERVER_H

#include "game.h"
#include "logger.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUMBER_OF_ROOMS 1024
#define MAX_CLIENTS 2

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
  time_t turn_max_timeval;
  pthread_mutex_t mutex;
};

struct ThreadInfo
{
  Room *rooms;
  Client client_a;
  Client client_b;
};

void run_server (const int port, const char *log_filename);

Board *get_board (Game *game, Player player);
Board *get_current_board (Game *game);
Board *get_opposing_board (Game *game);

bool is_room_available (Room *room);
bool should_room_finish (Room *room);
Client *get_client (Room *room, Player player);
Client *get_opposing_client (Room *room, Player player);
Client *get_current_client (Room *room);

void handle_message (Room *room, Client *client, char *message);
void notify_start_game (Room *room, Player player);
void *handle_game (void *arg);
void *handle_room (void *arg);

void send_to_client (Client *client, const char *message);
void multicast (const char *message, Room *room);

Room *search_available_room (Room *rooms);
void room_change_turn (Room *room);
time_t get_current_time ();
int recvtimeout (int sockfd, char *buf, int buflen, int timeout);
bool handshake (int client_fd);

#endif
