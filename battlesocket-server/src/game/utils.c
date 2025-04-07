#include "game.h"
#include <stdio.h>
#include <string.h>

// Returns a pointer to the board of `player`.
Board *
get_board (Game *game, Player player)
{
  switch (player)
    {
    case PLAYER_A:
      return &game->board_a;
    case PLAYER_B:
      return &game->board_b;
    default:
      return NULL;
    }
}

// Returns a pointer to the board of the current player.
Board *
get_current_board (Game *game)
{
  return get_board (game, game->current_player);
}

// Returns a pointer to the board of the opposite player (at the time being).
Board *
get_opposing_board (Game *game)
{
  switch (game->current_player)
    {
    case PLAYER_A:
      return &game->board_b;
    case PLAYER_B:
      return &game->board_a;
    default:
      return NULL;
    }
}

// Returns the corresponding ship at a certain (row, col).
// If either of the parameters exceeds the dimensions of the board, returns -1.
int
get_ship_index_at (Board *board, int row, int col)
{
  if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
    return -1;
  return board->ship_map[row][col];
}

// Returns an ASCII text representation of the board's ships in the format:
// "<ship_id>:<coord1> <coord2> ... ; <ship_id>:<coord1> ..."
void
get_ship_data (Board *board, char *buffer, size_t buffer_size)
{
  char temp[BUFSIZ] = "";
  int current_pos = 0;

  for (int i = 0; i < board->ship_count; i++)
    {
      Ship *s = &board->ships[i];

      // Create unique id using index.
      char ship_id[32];
      snprintf (ship_id, sizeof (ship_id), "%s%d", s->name, i + 1);
      char coords[128] = "";
      for (int j = 0; j < s->length; j++)
        {
          int r = s->start_row + (s->orientation == 1 ? j : 0);
          int c = s->start_col + (s->orientation == 0 ? j : 0);
          char cell[16];
          snprintf (cell, sizeof (cell), "%c%d", 'A' + r, c + 1);

          // Using snprintf.
          if (j == 0)
            {
              snprintf (coords, sizeof (coords), "%s", cell);
            }
          else
            {
              snprintf (coords + strlen (coords),
                        sizeof (coords) - strlen (coords), " %s", cell);
            }
        }
      // Using snprintf.
      int needed = snprintf (temp + current_pos, buffer_size - current_pos,
                             "%s:%s; ", ship_id, coords);

      if (needed < 0 || ((size_t)current_pos + (size_t)needed) >= buffer_size)
        break;
      current_pos += needed;
    }

  // Delete the last ";" if it exists.
  if (current_pos >= 2)
    {
      temp[current_pos - 2] = '\0';
    }

  strncpy (buffer, temp, buffer_size - 1);
  buffer[buffer_size - 1] = '\0';
}
