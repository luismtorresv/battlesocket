#include "protocol.h"
#include <stdio.h>
#include <string.h>

// Build string for START_GAME message.
void
build_start_game (char *buffer, Player player, const char *ship_data)
{
  sprintf (buffer, "START_GAME %c {%s}$", player, ship_data);
}

// Build string for response to action message (SHOT, in this case).
// - `was_hit` is a boolean. True if ship was hit.
// - `row` and `col` are the integer representations.
// - `sunk` is an int (interpreted as a bool) that indicates whether the shot
//   was a hit or miss
// - `current_player` is the player of the current turn, i.e., the one who
//   carried out the action.
void
build_action_result (char *buffer, const bool ship_was_hit, const int row,
                     const int col, int sunk)
{
  int row_as_char = row + 'A';
  int col_as_char = col + 1;
  const char *result = ship_was_hit ? "HIT" : "MISS";
  char *format = sunk ? "%s %c%d SUNK$" : "%s %c%d$";
  sprintf (buffer, format, result, row_as_char, col_as_char);
}

void
build_turn_msg (char *buffer, Player player, long timestamp)
{
  sprintf (buffer, "TURN %c %ld$", player, timestamp);
}

// Build string for END_GAME message.
void
build_end_game (char *buffer, Player winner)
{
  sprintf (buffer, "END_GAME %c$", winner);
}
