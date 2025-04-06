#include "server.h"
#include "game.h"
#include "logger.h"
#include "protocol.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUMBER_OF_ROOMS 1024
#define SERVER_PORT 8080
#define MAX_CLIENTS 2

// Local function declarations.
int init_server ();
void cleanup_server (int server_fd);

static Room ROOMS[NUMBER_OF_ROOMS] = { 0 };
static pthread_mutex_t room_mutex = PTHREAD_MUTEX_INITIALIZER;

// Sends a message to a client.
void
send_to_client (Client *client, const char *message)
{
  send (client->sockfd, message, strlen (message), 0);
  log_event (LOG_INFO, message);
}

// Sends a message to both clients of a room.
void
broadcast (const char *message, Room *room)
{
  send_to_client (&room->client_a, message);
  send_to_client (&room->client_b, message);
}

// Returns true if room is not occupied.
bool
is_room_available (Room *room)
{
  return (room->client_a.sockfd == 0) || (room->client_b.sockfd == 0);
}

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

// Returns the socket file descriptor of the current player.
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

// Returns a pointer to the client assigned to a certain player.
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

// Returns a pointer to the client of the current player.
Client *
get_current_client (Room *room)
{
  return get_client (room, room->game.current_player);
}

// Handle a message of the protocol.
void
handle_message (Room *room, const char *message)
{
  if (parse_message (message) != MSG_SHOT)
    {
      send_to_client (get_current_client (room), "BAD_REQUEST\n");
      return;
    }

  // We expect a message like "SHOT|A-1"
  char *separator = strchr (message, ' ');
  if (separator == NULL)
    {
      send_to_client (get_current_client (room), "BAD_REQUEST\n");
      return;
    }
  char pos[16] = { 0 };
  strncpy (pos, separator + 1, sizeof (pos) - 1);

  if (pos[0] < 'A' || pos[0] > 'J' || pos[1] != '-')
    {
      send_to_client (get_current_client (room), "BAD_REQUEST\n");
      return;
    }
  char row_char = pos[0];
  int row = row_char - 'A';
  int col = atoi (pos + 2) - 1;

  log_event (LOG_INFO, "Processing shot.");

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
  log_event (LOG_INFO, "Action message sent");
}

// Change current player to opposing player.
void
change_turn (Room *room)
{
  pthread_mutex_lock (&room_mutex);
  Player next_player
      = (room->game.current_player == PLAYER_A) ? PLAYER_B : PLAYER_A;
  room->game.current_player = next_player;
  pthread_mutex_unlock (&room_mutex);
}

// Send start game message to `player`.
void
send_start_game (Room *room, Player player)
{
  Board *board = get_board (&room->game, player);
  Client *client = get_client (room, player);
  Player initial_player = room->game.current_player;
  long int start_time = room->game.start_time;

  char ship_data[BUFSIZ] = { 0 };
  get_ship_data (board, ship_data, sizeof (ship_data));

  char start_message[BUFSIZ] = { 0 };
  build_start_game (start_message, start_time, initial_player, ship_data);
  send_to_client (client, start_message);
}

// Handle a client throughout the entire game session.
void *
handle_client (void *arg)
{
  // Copy of the argument.
  Client base_client = *(Client *)arg;
  free (arg);

  Client *client = NULL; // Will point to actual client in room.
  Room *room = NULL;
  Game *game = NULL;

  // Search for an available room.
  pthread_mutex_lock (&room_mutex);
  for (int i = 0; i < NUMBER_OF_ROOMS; ++i)
    {
      game = &ROOMS[i].game;
      if (game->state == WAITING || game->state == AVAILABLE)
        {
          room = &ROOMS[i];
          // Is first client unassigned?
          if (room->client_a.sockfd == 0)
            {
              room->client_a = base_client;
              room->client_a.player = PLAYER_A;
              client = &room->client_a;
              game->state = WAITING;
            }
          else
            {
              room->client_b = base_client;
              room->client_b.player = PLAYER_B;
              client = &room->client_b;
              game->state = READY_TO_START;
            }
          break;
        }
    }
  pthread_mutex_unlock (&room_mutex);

  if (room == NULL)
    {
      log_event (LOG_ERROR, "Server is full.");
      if (client)
        close (client->sockfd);
      return NULL;
    }

  // Send JOINED_MATCHMAKING
  log_event (LOG_INFO, "New client connected");
  char buffer[BUFSIZ];
  build_joined_matchmaking (buffer, client->player);
  send_to_client (client, buffer);

  while (game->state == WAITING)
    {
      sleep (1);
    }

  // pthread_mutex_lock (&room_mutex);
  if (game->state == READY_TO_START)
    {
      // Send to each client the START_GAME with their boards
      init_game (&room->game);
      send_start_game (room, PLAYER_A);
      send_start_game (room, PLAYER_B);
      game->state = IN_PROGRESS;
    }
  // pthread_mutex_unlock (&room_mutex);

  // After second client joins.
  char recv_buffer[BUFSIZ];
  while (!is_game_over (get_opposing_board (game)))
    {
      memset (recv_buffer, 0, sizeof (recv_buffer));
      int bytes_read
          = recv (client->sockfd, recv_buffer, sizeof (recv_buffer) - 1, 0);
      if (bytes_read == 0)
        {
          log_event (LOG_INFO, "Client disconnection.");
          break;
        }
      else if (bytes_read == 1)
        {
          log_event (LOG_ERROR, "Failed to recv data.");
          break;
        }

      int newline_pos = strcspn (recv_buffer, "\r\n");
      recv_buffer[newline_pos] = '\0';

      log_event (LOG_DEBUG, "Player message received");
      handle_message (room, recv_buffer);

      change_turn (room);
    }

  // Game over.
  if (is_game_over (get_opposing_board (game)))
    {
      char end_msg[BUFSIZ];
      build_end_game (end_msg,
                      room->game.current_player == PLAYER_A ? 'A' : 'B');
      broadcast (end_msg, room);
      log_event (LOG_INFO, "Game over");
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
      log_event (LOG_ERROR, "Failed to create socket.");
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
      log_event (LOG_ERROR, "[Error] Failed to set SO_REUSEADDR.");
      exit (1);
    }

  if (bind (server_fd, (struct sockaddr *)&serv_addr, sizeof (serv_addr))
      == -1)
    {
      log_event (LOG_ERROR, "[Error] Failed to bind.");
      exit (1);
    }

  if (listen (server_fd, MAX_CLIENTS) == -1)
    {
      log_event (LOG_ERROR, "Failed to listen.");
      exit (1);
    }

  log_event (LOG_INFO, "Server listening with file descriptor %d.", server_fd);

  for (int i = 0; i < NUMBER_OF_ROOMS; ++i)
    {
      ROOMS[i].id = i;
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
          log_event (LOG_ERROR, "Failed to create thread.");
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
  log_event (LOG_INFO, "Server closed ^_____^");

  // TODO: Close all sockets of all rooms.
}
