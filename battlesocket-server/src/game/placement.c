#include "game.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Generate a random integer between min and max.
static int
random_int (int min, int max)
{
  return min + rand () % (max - min + 1);
}

// Place a ship in a game board.
// - `name` is the type of ship.
// - `length` is the number of cells it occupies.
// Returns 1 if placed, 0 if it failed.
int
place_ship (Board *board, const char *name, int length)
{
  bool was_placed = false;
  for (int attempts = 0; !was_placed && attempts < 100; ++attempts)
    {
      int orientation, max_row, max_col, start_row, start_col;

      orientation = random_int (0, 1); // 0 horizontal, 1 vertical
      max_row = (orientation == 1) ? BOARD_SIZE - length : BOARD_SIZE - 1;
      max_col = (orientation == 0) ? BOARD_SIZE - length : BOARD_SIZE - 1;
      start_row = random_int (0, max_row);
      start_col = random_int (0, max_col);

      // Check if the cells are free
      bool is_free = true;
      for (int i = 0; i < length; i++)
        {
          int r = start_row + (orientation == 1 ? i : 0);
          int c = start_col + (orientation == 0 ? i : 0);
          if (board->ship_map[r][c] != -1)
            {
              is_free = false;
              break;
            }
        }
      if (!is_free)
        continue;

      // Placing the ship
      board->ships[board->ship_count] = (Ship){
        .name = name,
        .length = length,
        .orientation = orientation,
        .start_row = start_row,
        .start_col = start_col,
        .hits = 0,
      };

      // Update grid and ship_map
      for (int i = 0; i < length; i++)
        {
          int r = start_row + (orientation == 1 ? i : 0);
          int c = start_col + (orientation == 0 ? i : 0);
          board->grid[r][c] = SHIP;
          board->ship_map[r][c] = board->ship_count;
        }

      ++board->ship_count;
      was_placed = true;
    }

  return was_placed;
}

// Place ships in a game board.
void
place_ships (Board *board)
{
  // 1 Carrier (5),
  // 1 Battleship (4),
  // 2 Cruisers (3),
  // 2 Destroyers (2),
  // 3 Submarines (1)

  // Carrier
  if (!place_ship (board, "carrier", 5))
    {
      fprintf (stderr, "Error placing carrier\n");
    }
  // Battleship
  if (!place_ship (board, "battleship", 4))
    {
      fprintf (stderr, "Error placing battleship\n");
    }

  // 2 Cruisers
  if (!place_ship (board, "cruiser", 3))
    {
      fprintf (stderr, "Error placing cruiser\n");
    }
  if (!place_ship (board, "cruiser", 3))
    {
      fprintf (stderr, "Error placing cruiser\n");
    }

  // 2 Destroyers
  if (!place_ship (board, "destroyer", 2))
    {
      fprintf (stderr, "Error placing destroyer\n");
    }
  if (!place_ship (board, "destroyer", 2))
    {
      fprintf (stderr, "Error placing destroyer\n");
    }

  // 3 Submarines
  if (!place_ship (board, "submarine", 1))
    {
      fprintf (stderr, "Error placing submarine\n");
    }
  if (!place_ship (board, "submarine", 1))
    {
      fprintf (stderr, "Error placing submarine\n");
    }
  if (!place_ship (board, "submarine", 1))
    {
      fprintf (stderr, "Error placing submarine\n");
    }
}
