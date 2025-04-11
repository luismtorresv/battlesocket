#include "game.h"
#include "logger.h"
#include <stdlib.h>
#include <time.h>

// Initialise game.
void
init_game (Game *game)
{
  if (game->state != READY_TO_START)
    return;

  init_board (&game->board_a);
  init_board (&game->board_b);

  Player starting_player = choose_starting_player (game);

  log_event (LOG_INFO, "Initialised game with id: %d, starting player: %c.",
             game->id, starting_player);
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
  return (game->current_player
          = (rand () % 2 == 0)
                ? PLAYER_A
                : PLAYER_B); // If the number it's even, A goes first.
}
