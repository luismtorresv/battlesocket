#include "protocol.h"
#include <stdio.h>
#include <string.h>

// Parse client message and return message type.
MsgType
parse_message (const char *msg)
{
  if (strncmp (msg, "SHOT", 4) == 0)
    return MSG_SHOT;
  return MSG_BAD_REQUEST;
}

// Build string for START_GAME message.
void
build_start_game (char *buffer, long unix_time, Player initial_player,
                  const char *ship_data)
{
  sprintf (buffer, "START_GAME start_time:%ld initial_player:%c board:{%s}\n",
           unix_time, initial_player, ship_data);
}

// Build string for JOINED_MATCHMAKING message.
void
build_joined_matchmaking (char *buffer, Player player)
{
  sprintf (buffer, "JOINED_MATCHMAKING %c\n", player);
}

// Build string for response to action message (SHOT, in this case).
// - `result` is a string that can be either "HIT" or "MISS".
// - `pos` is the (row,col) representation of the cell.
// - `sunk` is an int (interpreted as a bool) that indicates whether the shot
//   was a hit or miss
// - `current_player` is the player of the current turn, i.e., the one who
//   carried out the action.
void
build_action_result (char *buffer, const char *result, const char *pos,
                     int sunk, const Player current_player)
{
  char next_player = current_player == PLAYER_A ? PLAYER_B : PLAYER_A;
  char *format = sunk ? "%s %s SUNK next:%c\n" : "%s %s next:%c\n";
  sprintf (buffer, format, result, pos, next_player);
}

// Build string for response to a bad request message.
void
build_bad_request (char *buffer, const Player current_player)
{
  const char *prefix = "BAD_REQUEST";
  char next_player = current_player == PLAYER_A ? PLAYER_B : PLAYER_A;
  sprintf (buffer, "%s next:%c\n", prefix, next_player);
}

// Build string for END_GAME message.
void
build_end_game (char *buffer, Player winner)
{
  sprintf (buffer, "END_GAME %c\n", winner);
}
