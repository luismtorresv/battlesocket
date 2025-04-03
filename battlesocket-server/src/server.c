#include "server.h"
#include "common.h"
#include "game.h"
#include "logger.h"
#include "protocol.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUMBER_OF_ROOMS 1024
#define SERVER_PORT 8080
#define MAX_CLIENTS 2

typedef struct Client Client;
typedef struct Room Room;
typedef struct Game Game;

struct Client
{
  int sockfd;
  struct sockaddr_in addr;
  Room *room;
};

struct Game
{
  Board board_a;
  Board board_b;
  Player current_player;
};

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

Board *
get_current_board (Game *game)
{
  return get_board (game, game->current_player);
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
get_client (Room *room, Player player)
{
  switch (player)
    {
    case PLAYER_A:
      return &room->client_a;
    case PLAYER_B:
      return &room->client_b;
    default:
      return NULL;
    }
}

Client *
get_current_client (Room *room)
{
  return get_client (room, room->game.current_player);
}

void
handle_message (Room *room, const char *message)
{
  if (parse_message (message) != MSG_SHOT)
    {
      send_to_client (get_current_client (room), "BAD_REQUEST|\n");
      return;
    }

  // We expect a message like "SHOT|A-1"
  char *separator = strchr (message, '|');
  if (separator == NULL)
    {
      send_to_client (get_current_client (room), "BAD_REQUEST|\n");
      return;
    }
  char pos[16] = { 0 };
  strncpy (pos, separator + 1, sizeof (pos) - 1);

  if (pos[0] < 'A' || pos[0] > 'J' || pos[1] != '-')
    {
      send_to_client (get_current_client (room), "BAD_REQUEST|\n");
      return;
    }
  char row_char = pos[0];
  int row = row_char - 'A';
  int col = atoi (pos + 2) - 1;

  log_event ("Processing shot...");

  // The shot happens in the board of the opposing player
  int hit = validate_shot (get_opposing_board (&room->game), row, col);
  update_board (get_opposing_board (&room->game), row, col, hit);

  int sunk = 0;
  if (hit)
    {
      int ship_index
          = get_ship_index_at (get_opposing_board (&room->game), row, col);
      if (ship_index != -1)
        sunk = is_ship_sunk (get_opposing_board (&room->game), ship_index);
    }

  const char *result = hit ? "HIT" : "MISS";
  char action_msg[BUFSIZ];
  memset (action_msg, 0, sizeof (action_msg));
  build_action_result (action_msg, result, pos, sunk,
                       room->game.current_player);
  broadcast (action_msg, room);
  log_event ("Action message sent");
}

// Decides starting player using the stdlib's random number generator.
Player
choose_starting_player (Room *room)
{
  srand (time (NULL)); // Set seed using current time.
  return (room->game.current_player
          = (rand () % 2 == 0)
                ? PLAYER_A
                : PLAYER_B); // If the number it's even, A goes first.
}

void
send_start_game (Room *room, Player player, long start_time)
{
  Board *board = get_board (&room->game, player);
  Client *client = get_client (room, player);
  Player initial_player = room->game.current_player;

  char ship_data[BUFSIZ] = { 0 };
  get_ship_data (board, ship_data, sizeof (ship_data));

  char start_message[BUFSIZ] = { 0 };
  build_start_game (start_message, start_time, initial_player, ship_data);
  send_to_client (client, start_message);
  log_event (start_message);
}

void
init_game (Room *room)
{
  // Initialising the game for the single room.
  // This is supposed to be done for every match.
  init_board (&room->game.board_a);
  init_board (&room->game.board_b);
  place_ships (&room->game.board_a);
  place_ships (&room->game.board_b);

  const long int START_GAME_DELAY = 5; // Units: seconds.
  long start_time = time (NULL) + START_GAME_DELAY;

  choose_starting_player (room);

  // Send to each client the START_GAME with their boards
  send_start_game (room, PLAYER_A, start_time);
  send_start_game (room, PLAYER_B, start_time);

  log_event ("[INFO] Game started.");
}

void *
handle_client (void *arg)
{
  Client client = *(Client *)arg;
  free (arg);

  int room_index = -1;

  // Search for an available room.
  for (int i = 0; i < NUMBER_OF_ROOMS; ++i)
    {
      if (rooms[i].is_available)
        {
          // Is first client unassigned?
          if (!rooms[i].client_a.sockfd)
            rooms[i].client_a = client;
          else
            rooms[i].client_b = client;
          room_index = i;
          break;
        }
    }

  // Send JOINED_MATCHMAKING
  log_event ("New client connected");
  char buffer[BUFSIZ];
  build_joined_matchmaking (buffer,
                            rooms[room_index].client_a.sockfd ? 'A' : 'B');
  send_to_client (&client, buffer);

  while (!client.room->client_b.sockfd)
    {
    }

  rooms[room_index].is_available = false;
  rooms[room_index].start_time = time (NULL);

  // After second client joins.
  init_game (&rooms[room_index]);
  char recv_buffer[BUFSIZ];
  while (!is_game_over (get_opposing_board (&rooms[room_index].game)))
    {
      memset (recv_buffer, 0, sizeof (recv_buffer));
      int bytes_read = recv (get_current_socket_fd (&rooms[room_index]),
                             recv_buffer, sizeof (recv_buffer) - 1, 0);
      if (bytes_read == 0)
        {
          log_event ("[ERROR] Client disconnection.");
          break;
        }
      else if (bytes_read == 1)
        {
          log_event ("[ERROR] Failed to recv data.");
          break;
        }

      int newline_pos = strcspn (recv_buffer, "\r\n");
      recv_buffer[newline_pos] = '\0';

      log_event ("Player message received");
      handle_message (&rooms[room_index], recv_buffer);

      // Player turn change
      rooms[room_index].game.current_player
          = (rooms[room_index].game.current_player == PLAYER_A) ? PLAYER_B
                                                                : PLAYER_A;
    }

  // Game over.
  if (is_game_over (get_opposing_board (&rooms[room_index].game)))
    {
      char end_msg[BUFSIZ];
      build_end_game (end_msg,
                      rooms[room_index].game.current_player == PLAYER_A ? 'A'
                                                                        : 'B');
      broadcast (end_msg, &rooms[room_index]);
      log_event ("Game over");
    }

  return NULL;
}

// Initialise server with its socket.
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

  for (int i = 0; i < NUMBER_OF_ROOMS; ++i)
    {
      rooms[i].is_available = true;
    }

  return server_fd;
}

// Accept incoming client connections and dispatch them.
void
run_server ()
{
  int server_fd = init_server ();
  int client_socket;

  struct sockaddr_in client_addr;
  socklen_t client_addr_len;
  client_addr_len = sizeof (client_addr);

  while ((client_socket = accept (server_fd, (struct sockaddr *)&client_addr,
                                  &client_addr_len))
         != -1)
    {
      pthread_t thread_id;

      // Set up the argument to the thread.
      // We care about the address ('cos logs), so we store it instead of
      // calling `getsockname` each. damn. time.
      Client client;
      client.addr = client_addr;
      client.sockfd = client_socket;
      Client *new_client = malloc (sizeof (Client));
      *new_client = client;

      // Create the actual goddamn thread.
      if (pthread_create (&thread_id, NULL, handle_client, (void *)new_client)
          != 0)
        {
          fprintf (stderr, "[ERROR] Failed to create thread.\n");
          free (new_client);
        }
      pthread_detach (thread_id);
    }

  cleanup_server (server_fd);
}

// Close socket file descriptors.
void
cleanup_server (int server_fd)
{
  close (server_fd);
  log_event ("Server closed ^_____^");

  // TODO: Close all sockets of all rooms.
}
