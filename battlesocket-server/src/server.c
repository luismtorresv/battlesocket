#include "server.h"
#include "common.h"
#include "game.h"
#include "logger.h"
#include "protocol.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUMBER_OF_ROOMS 1024
#define SERVER_PORT 8080
#define MAX_CLIENTS 2

typedef struct
{
  int sockfd;
  struct sockaddr_in addr;
} Client;

typedef struct Game Game;
struct Game
{
  Board board_a;
  Board board_b;
  Player current_player;
};

typedef struct Room Room;
struct Room
{
  size_t id;
  Client client_a;
  Client client_b;
  time_t start_time;
  bool is_available;
  Game game;
};

// Local function declarations.
int init_server ();
void cleanup_server (int server_fd);

Room rooms[NUMBER_OF_ROOMS] = { 0 };
Room *single_room = &rooms[0];

void
send_to_client (Client *client, const char *message)
{
  send (client->sockfd, message, strlen (message), 0);
}

// Juan Carlos: "Broadca'ting"
void
broadcast (const char *message, Room *room)
{
  send_to_client (&room->client_a, message);
  send_to_client (&room->client_b, message);
}

int
init_server ()
{
  int server_fd;
  if ((server_fd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      fprintf (stderr, "[Error] Failed to create socket.\n");
      exit (1);
    }

  struct sockaddr_in serv_addr = {
    .sin_family = AF_INET,
    .sin_port = htons (SERVER_PORT),
    .sin_addr = { htonl (INADDR_ANY) },
  };

  int reuse = 1;
  if (setsockopt (server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse))
      == -1)
    {
      fprintf (stderr, "[Error] Failed to set SO_REUSEADDR.\n");
      exit (1);
    }

  if (bind (server_fd, (struct sockaddr *)&serv_addr, sizeof (serv_addr))
      == -1)
    {
      fprintf (stderr, "[Error] Failed to bind.\n");
      exit (1);
    }

  if (listen (server_fd, MAX_CLIENTS) == -1)
    {
      fprintf (stderr, "[Error] Failed to listen.\n");
      exit (1);
    }

  log_event ("Server initialized and listening...");
  printf ("Server created with fd %d\n", server_fd);
  return server_fd;
}

Board *
get_board (Game *game, Player *player)
{
  switch (*player)
    {
    case PLAYER_A:
      return &game->board_a;
    case PLAYER_B:
      return &game->board_b;
    default:
      return NULL;
    }
}

Board *
get_current_board (Game *game)
{
  return get_board (game, &game->current_player);
}

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

int
get_current_socket_fd (Room *room)
{
  switch (room->game.current_player)
    {
    case PLAYER_A:
      return room->client_a.sockfd;
    case PLAYER_B:
      return room->client_b.sockfd;
    default:
      return -1;
    }
}

Client *
get_current_client (Room *room)
{
  switch (room->game.current_player)
    {
    case PLAYER_A:
      return &room->client_a;
    case PLAYER_B:
      return &room->client_b;
    default:
      return NULL;
    }
}

void
handle_message (const char *message)
{
  if (parse_message (message) != MSG_SHOT)
    {
      send_to_client (get_current_client (single_room), "BAD_REQUEST|\n");
      return;
    }

  // We expect a message like "SHOT|A-1"
  char *separator = strchr (message, '|');
  if (separator == NULL)
    {
      send_to_client (get_current_client (single_room), "BAD_REQUEST|\n");
      return;
    }
  char pos[16] = { 0 };
  strncpy (pos, separator + 1, sizeof (pos) - 1);

  if (pos[0] < 'A' || pos[0] > 'J' || pos[1] != '-')
    {
      send_to_client (get_current_client (single_room), "BAD_REQUEST|\n");
      return;
    }
  char row_char = pos[0];
  int row = row_char - 'A';
  int col = atoi (pos + 2) - 1;

  log_event ("Processing shot...");

  // The shot happens in the board of the opposing player
  int hit = validate_shot (get_opposing_board (&single_room->game), row, col);
  update_board (get_opposing_board (&single_room->game), row, col, hit);

  int sunk = 0;
  if (hit)
    {
      int ship_index = get_ship_index_at (
          get_opposing_board (&single_room->game), row, col);
      if (ship_index != -1)
        sunk = is_ship_sunk (get_opposing_board (&single_room->game),
                             ship_index);
    }

  const char *result = hit ? "HIT" : "MISS";
  char action_msg[BUFSIZ];
  memset (action_msg, 0, sizeof (action_msg));
  build_action_result (action_msg, result, pos, sunk,
                       single_room->game.current_player);
  broadcast (action_msg, single_room);
  log_event ("Action message sent");
}

void
play_game (int server_fd)
{
  long start_time = time (NULL) + 5;
  srand (time (NULL));
  single_room->game.current_player = (rand () % 2 == 0) ? PLAYER_A : PLAYER_B;

  // Send to each client the START_GAME with their boards
  for (int i = 0; i < MAX_CLIENTS; i++)
    {
      char ship_data[1024] = "";
      Board *board
          = (i == 0) ? &single_room->game.board_a : &single_room->game.board_b;
      Client *client
          = (i == 0) ? &single_room->client_a : &single_room->client_b;
      get_ship_data (board, ship_data, sizeof (ship_data));
      char start_msg[2048] = "";
      // "START_GAME|<unix_time> <initial_player> <ship_data>\n"
      build_start_game (start_msg, start_time,
                        single_room->game.current_player, ship_data);
      send_to_client (client, start_msg);
      log_event (start_msg);
    }

  log_event ("Game started...");

  bool game_over = false;
  char recv_buffer[BUFSIZ];
  while (!game_over)
    {
      memset (recv_buffer, 0, sizeof (recv_buffer));
      int bytes_read = recv (get_current_socket_fd (single_room), recv_buffer,
                             sizeof (recv_buffer) - 1, 0);
      if (bytes_read <= 0)
        {
          log_event ("Failed receiving data or client disconnection");
          break;
        }
      int newline_pos = strcspn (recv_buffer, "\r\n");
      recv_buffer[newline_pos] = '\0';

      log_event ("Player message received");
      handle_message (recv_buffer);

      // Check if the other player still stands (˘･_･˘)
      if (is_game_over (get_opposing_board (&single_room->game)))
        {
          char end_msg[BUFSIZ];
          build_end_game (end_msg, single_room->game.current_player == PLAYER_A
                                       ? 'A'
                                       : 'B');
          broadcast (end_msg, single_room);
          game_over = true;
          log_event ("Game over");
          break;
        }
      // Player turn change
      single_room->game.current_player
          = (single_room->game.current_player == PLAYER_A) ? PLAYER_B
                                                           : PLAYER_A;
    }

  cleanup_server (server_fd);
}

void
run_server ()
{
  int server_fd = init_server ();

  // First client to connect.
  bool is_client_a = true;
  Client *client = &single_room->client_a;
  while (single_room->client_a.sockfd == 0
         || single_room->client_b.sockfd == 0)
    {
      if (!is_client_a)
        client = &single_room->client_b;
      struct sockaddr_in client_addr;
      socklen_t client_addr_len;
      client_addr_len = sizeof (client_addr);

      int new_socket = accept (server_fd, (struct sockaddr *)&client_addr,
                               &client_addr_len);
      if (new_socket < 0)
        {
          log_event ("Error accepting connection");
          continue;
        }

      client->sockfd = new_socket;
      client->addr = client_addr;
      log_event ("New client connected");

      // Send JOINED_MATCHMAKING
      char buffer[BUFSIZ];
      build_joined_matchmaking (buffer, is_client_a ? 'A' : 'B');
      send_to_client (client, buffer);

      single_room->is_available = false;
      single_room->is_available = time (NULL);
      is_client_a = false;
    }

  // Initialising the game for the single room.
  // This is supposed to be done for every match.
  init_board (&single_room->game.board_a);
  init_board (&single_room->game.board_b);
  place_ships (&single_room->game.board_a);
  place_ships (&single_room->game.board_b);

  play_game (server_fd);
}

void
cleanup_server (int server_fd)
{
  close (server_fd);

  // TODO: Close all sockets of all rooms.
  close (single_room->client_a.sockfd);
  close (single_room->client_b.sockfd);
  log_event ("Server closed");
}
