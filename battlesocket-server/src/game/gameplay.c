#include "game.h"

// Change current player to opposing player.
void
change_turn (Game *game)
{
  game->current_player
      = (game->current_player == PLAYER_A) ? PLAYER_B : PLAYER_A;
}

// Validate a shot in `row` and `col`.
// Return 1 if cell in there's a ship in (row,col) and 0 otherwise.
int
validate_shot (Board *board, int row, int col)
{
  if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
    return 0;
  if (board->grid[row][col] == SHIP)
    return 1;
  return 0;
}

// Returns 1 if all the cells of a ship have been hit and 0 otherwise.
int
is_ship_sunk (Board *board, int ship_index)
{
  if (ship_index < 0 || ship_index >= board->ship_count)
    return 0;
  return (board->ships[ship_index].hits == board->ships[ship_index].length);
}

// Update board at (row,col) with a hit.
void
update_board (Board *board, int row, int col, int hit)
{
  if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
    return;
  if (hit)
    {
      board->grid[row][col] = HIT;
      int ship_index = board->ship_map[row][col];
      if (ship_index != -1)
        board->ships[ship_index].hits++;
    }
  else
    {
      board->grid[row][col] = MISS;
    }
}

// Returns 1 if all ships in a board are sunk, 0 otherwise.
int
is_game_over (Board *board)
{
  for (int i = 0; i < board->ship_count; i++)
    {
      if (board->ships[i].hits < board->ships[i].length)
        return 0;
    }
  return 1;
}
