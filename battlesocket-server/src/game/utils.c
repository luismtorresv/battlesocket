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
// Returns NULL if it's out of board bounds.
Ship *
get_ship (Board *board, int row, int col)
{
  if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
    return NULL;
  int ship_index = board->ship_map[row][col];
  return &board->ships[ship_index];
}

// Returns an ASCII text representation of the board's ships in the format:
// "<ship_id>:<coord1> <coord2> ... ; <ship_id>:<coord1> ..."
void
get_ship_data (Board *board, char *buffer, size_t buffer_size)
{
  memset (buffer, 0, buffer_size);
  for (int i = 0; i < board->ship_count; i++)
    {
      Ship *ship = &board->ships[i];

      char coords[128] = { 0 };
      for (int j = 0; j < ship->length; j++)
        {
          int row = ship->start_row + (ship->orientation == 1 ? j : 0);
          int col = ship->start_col + (ship->orientation == 0 ? j : 0);
          char cell[16];
          snprintf (cell, sizeof (cell),
                    (j != ship->length - 1) ? "%c%d " : "%c%d", 'A' + row,
                    col + 1);
          strncat (coords, cell, sizeof (coords) - strlen (coords) - 1);
        }

      char name_plus_coords[128 + 32 + 1] = { 0 };
      snprintf (name_plus_coords, sizeof (name_plus_coords), "%s:%s;",
                ship->name, coords);
      strncat (buffer, name_plus_coords, buffer_size);
    }
}
