#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stddef.h>

#define BOARD_SIZE 10
#define NUM_SHIPS 9

typedef enum
{
  NO_PLAYER = 0,
  PLAYER_A = 'A',
  PLAYER_B = 'B',
} Player;

typedef enum
{
  AVAILABLE = 0,
  WAITING,
  READY_TO_START,
  IN_PROGRESS,
  FINISHED = AVAILABLE,
} GAME_STATE;

typedef enum
{
  WATER,
  SHIP,
  HIT,
  MISS
} CellStatus;

typedef struct Ship Ship;
typedef struct Board Board;
typedef struct Game Game;

struct Ship
{
  const char *name;
  int length;
  int orientation;
  int start_row;
  int start_col;
  int hits;
};

struct Board
{
  CellStatus grid[BOARD_SIZE][BOARD_SIZE];
  int ship_map[BOARD_SIZE][BOARD_SIZE];
  Ship ships[NUM_SHIPS];
  int ship_count;
};

struct Game
{
  int id;
  GAME_STATE state;
  Player current_player;
  Board board_a;
  Board board_b;
};

void init_game (Game *game);
void init_board (Board *board);
Player choose_starting_player (Game *game);

void change_turn (Game *game);

int place_ship (Board *board, const char *name, int length, int ship_index);
void place_ships (Board *board);

bool is_valid_shot (int row, int col);
bool was_ship_hit (Board *board, int row, int col);
int is_ship_sunk (Board *board, int ship_index);
void update_board (Board *board, int row, int col, int hit);
int is_game_over (Board *board);

int get_ship_index_at (Board *board, int row, int col);
void get_ship_data (Board *board, char *buffer, size_t buffer_size);
#endif
