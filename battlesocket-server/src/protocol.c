#include "protocol.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

MsgType
parse_message (const char *msg)
{
  if (strncmp (msg, "SHOT", 4) == 0)
    return MSG_SHOT;
  return MSG_BAD_REQUEST;
}

void
build_start_game (char *buffer, long unix_time, Player initial_player,
                  const char *ship_data)
{
  sprintf (buffer, "START_GAME start_time:%ld initial_player:%c %s\n",
           unix_time, initial_player, ship_data);
}

void
build_joined_matchmaking (char *buffer, Player player)
{
  sprintf (buffer, "JOINED_MATCHMAKING %c\n", player);
}

void
build_action_result (char *buffer, const char *result, const char *pos,
                     int sunk, const Player current_player)
{
  char next_player = current_player == PLAYER_A ? PLAYER_B : PLAYER_A;
  char *format = sunk ? "%s %s SUNK %c\n" : "%s %s %c\n";
  sprintf (buffer, format, result, pos, next_player);
}

void
build_shot (char *buffer, const char *pos)
{
  sprintf (buffer, "SHOT %s\n", pos);
}

void
build_end_game (char *buffer, const char winner)
{
  sprintf (buffer, "END_GAME %c\n", winner);
}
