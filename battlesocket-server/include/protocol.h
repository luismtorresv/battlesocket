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
void build_start_game (char *buffer, long unix_time, Player initial_player,
                       const char *ship_data);
void build_joined_matchmaking (char *buffer, Player letter);
void build_action_result (char *buffer, const char *result, const char *pos,
                          int sunk, const Player current_player);
void build_bad_request(char *buffer, const char *badrequest, const Player current_player);
void build_shot (char *buffer, const char *pos);
void build_end_game (char *buffer, Player winner);

#endif
