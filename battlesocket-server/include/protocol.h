#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "server.h"

#define TERMINATOR "\n"

typedef enum
{
  MSG_SHOT,
  MSG_BAD_REQUEST,
  MSG_SURRENDER,
} MessageType;

typedef enum
{
  DISCONNECTION = 0,
  WINNER,
  SURRENDER,
  TIMEOUT,
} EndGameReason;

typedef struct Shot Shot;

struct Shot
{
  bool is_valid_shot;
  Player player;
  int row;
  int col;
};

MessageType parse_message (const char *message);
Shot parse_shot (const char *message);

void build_start_game (char *buffer, Player player, const char *ship_data);
void build_action_result (char *buffer, const bool ship_was_hit, const int row,
                          const int col, int sunk);
void build_turn_msg (char *buffer, Player player, long timestamp);

void send_bad_request (Client *client);
void send_start_game (Client *client, Player player, const char *ship_data);
void send_joined_matchmaking (Client *client);

void multicast_current_turn (Room *room);
void multicast_end_game (Room *room, Player player, EndGameReason reason);

#endif
