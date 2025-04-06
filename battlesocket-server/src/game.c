#include "game.h"
#include "logger.h"
#include "server.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Generate a random integer between min and max.
static int
random_int (int min, int max)
{
  return min + rand () % (max - min + 1);
}

// Initialise game.
void
init_game (Game *game)
{
  if (game->state != READY_TO_START)
    return;

  init_board (&game->board_a);
  init_board (&game->board_b);

  const long int START_GAME_DELAY = 5; // Units: seconds.
  game->start_time = time (NULL) + START_GAME_DELAY;

  choose_starting_player (game);

  log_event (LOG_INFO, "Initialised game %d.", game->id);
}

// Initialise game board.
void
init_board (Board *board)
{
  for (int i = 0; i < BOARD_SIZE; i++)
    {
      for (int j = 0; j < BOARD_SIZE; j++)
        {
          board->grid[i][j] = WATER;
          board->ship_map[i][j] = -1;
        }
    }
  board->ship_count = 0;

  place_ships (board);
}

// Decides starting player using the stdlib's random number generator.
Player
choose_starting_player (Game *game)
{
  srand (time (NULL)); // Set seed using current time.
  return (game->current_player
          = (rand () % 2 == 0)
                ? PLAYER_A
                : PLAYER_B); // If the number it's even, A goes first.
}

// Change current player to opposing player.
void
change_turn (Game *game)
{
  game->current_player
      = (game->current_player == PLAYER_A) ? PLAYER_B : PLAYER_A;
  ;
}

// Place a ship in a game board.
// - `name` is the type of ship.
// - `length` is the number of cells it occupies.
// - `ship_index` is the index in the `ships` member of `Board`.
// Returns 0 if placed, -1 if it failed.
int
place_ship (Board *board, const char *name, int length, int ship_index)
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
      board->ships[ship_index].name = name;
      board->ships[ship_index].length = length;
      board->ships[ship_index].orientation = orientation;
      board->ships[ship_index].start_row = start_row;
      board->ships[ship_index].start_col = start_col;
      board->ships[ship_index].hits = 0;

      // Update grid and ship_map
      for (int i = 0; i < length; i++)
        {
          int r = start_row + (orientation == 1 ? i : 0);
          int c = start_col + (orientation == 0 ? i : 0);
          board->grid[r][c] = SHIP;
          board->ship_map[r][c] = ship_index;
        }
      was_placed = true;
    }

  if (!was_placed)
    return -1; // Couldn't be placed

  board->ship_count++;
  return 0;
}

// Place ships in a game board.
void
place_ships (Board *board)
{
  // 1 Carrier (5), 1 Battleship (4), 2 Cruisers (3), 2 Destroyers (2), 3
  // Submarines (1)
  int ship_index = 0;
  // Carrier
  if (place_ship (board, "carrier", 5, ship_index) == -1)
    {
      perror ("Error placing carrier");
    }
  ship_index++;
  // Battleship
  if (place_ship (board, "battleship", 4, ship_index) == -1)
    {
      perror ("Error placing battleship");
    }
  ship_index++;
  // 2 Cruisers
  if (place_ship (board, "cruiser", 3, ship_index) == -1)
    {
      perror ("Error placing cruiser1");
    }
  ship_index++;
  if (place_ship (board, "cruiser", 3, ship_index) == -1)
    {
      perror ("Error placing cruiser2");
    }
  ship_index++;
  // 2 Destroyers
  if (place_ship (board, "destroyer", 2, ship_index) == -1)
    {
      perror ("Error placing destroyer1");
    }
  ship_index++;
  if (place_ship (board, "destroyer", 2, ship_index) == -1)
    {
      perror ("Error placing destroyer2");
    }
  ship_index++;
  // 3 Submarines
  if (place_ship (board, "submarine", 1, ship_index) == -1)
    {
      perror ("Error placing submarine1");
    }
  ship_index++;
  if (place_ship (board, "submarine", 1, ship_index) == -1)
    {
      perror ("Error placing submarine2");
    }
  ship_index++;
  if (place_ship (board, "submarine", 1, ship_index) == -1)
    {
      perror ("Error placing submarine3");
    }
  ship_index++;
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
  int counts[26] = { 0 }; // This is to count how many ships of each type,
                          // using the first letter of the name as a key
  for (int i = 0; i < board->ship_count; i++)
    {
      Ship *s = &board->ships[i];
      int idx = s->name[0] - 'a';
      counts[idx]++;
      char ship_id[32];
      snprintf (ship_id, sizeof (ship_id), "%s%d", s->name, counts[idx]);
      char coords[128] = "";
      for (int j = 0; j < s->length; j++)
        {
          int r = s->start_row + (s->orientation == 1 ? j : 0);
          int c = s->start_col + (s->orientation == 0 ? j : 0);
          char cell[16];
          snprintf (cell, sizeof (cell), "%c%d", 'A' + r, c + 1);
          strcat (coords, cell);
          if (j < s->length - 1)
            strcat (coords, " ");
        }
      strcat (temp, ship_id);
      strcat (temp, ":");
      strcat (temp, coords);
      if (i < board->ship_count - 1)
        strcat (temp, "; ");
    }
  strncpy (buffer, temp, buffer_size - 1);
  buffer[buffer_size - 1] = '\0';
}
