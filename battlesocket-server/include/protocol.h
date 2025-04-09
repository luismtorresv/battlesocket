#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "server.h"

typedef enum
{
  MSG_START_GAME,
  MSG_ACTION_RESULT,
  MSG_SHOT,
  MSG_JOINED_MATCHMAKING,
  MSG_END_GAME,
  MSG_BAD_REQUEST
} MsgType;

MsgType parse_message (const char *msg);
void build_start_game (char *buffer, Player player, const char *ship_data);
void build_action_result (char *buffer, const bool ship_was_hit, const char *pos, int sunk);
void build_turn_msg (char *buffer, Player player, long timestamp);
void build_end_game (char *buffer, Player winner);

void send_bad_request (Client *client);
void send_start_game (Client *client, Player player, const char *ship_data);
void send_end_game (Client *client, Player winner);
void send_joined_matchmaking (Client *client);

#endif
