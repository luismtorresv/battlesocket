#ifndef GAME_H
#define GAME_H

#include <stddef.h>

#define BOARD_SIZE 10
#define NUM_SHIPS 9

typedef enum
{
  WATER,
  SHIP,
  HIT,
  MISS
} CellStatus;

typedef struct
{
  const char *name;
  int length;
  int orientation;
  int start_row;
  int start_col;
  int hits;
} Ship;

typedef struct
{
  CellStatus grid[BOARD_SIZE][BOARD_SIZE];
  int ship_map[BOARD_SIZE][BOARD_SIZE];
  Ship ships[NUM_SHIPS];
  int ship_count;
} Board;

void init_board (Board *board);
int place_ship (Board *board, const char *name, int length, int ship_index);
void place_ships (Board *board);
int validate_shot (Board *board, int row, int col);
void update_board (Board *board, int row, int col, int hit);
int is_game_over (Board *board);
int is_ship_sunk (Board *board, int ship_index);
int get_ship_index_at (Board *board, int row, int col);
void get_ship_data (Board *board, char *buffer, size_t buffer_size);

#endif