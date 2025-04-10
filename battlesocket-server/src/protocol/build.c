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
build_start_game (char *buffer, Player player, const char *ship_data)
{
  sprintf (buffer, "START_GAME %c {%s}$", player, ship_data);
}

// Build string for response to action message (SHOT, in this case).
// - `was_hit` is a boolean. True if ship was hit.
// - `pos` is the (row,col) representation of the cell.
// - `sunk` is an int (interpreted as a bool) that indicates whether the shot
//   was a hit or miss
// - `current_player` is the player of the current turn, i.e., the one who
//   carried out the action.
void
build_action_result (char *buffer, const bool ship_was_hit, const char *pos, int sunk)
{
  const char *result = ship_was_hit ? "HIT" : "MISS";
  char *format = sunk ? "%s %s SUNK$" : "%s %s$";
  sprintf (buffer, format, result, pos);
}

void build_turn_msg(char *buffer, Player player, long timestamp) 
{
  sprintf(buffer, "TURN %c %ld$", player, timestamp);
}

// Build string for END_GAME message.
void
build_end_game (char *buffer, Player winner)
{
  sprintf (buffer, "END_GAME %c$", winner);
}
