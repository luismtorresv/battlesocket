#include "server.h"

// Handle a message of the protocol.
void
handle_message (Room *room, Client *client, char *message)
{
  int newline_pos = strcspn (message, "\r\n");
  message[newline_pos] = '\0';

  if (parse_message (message) != MSG_SHOT)
    {
      send_to_client (client, "BAD_REQUEST\n");
      return;
    }

  // We expect a message like "SHOT|A-1"
  char *separator = strchr (message, ' ');
  if (separator == NULL)
    {
      send_to_client (client, "BAD_REQUEST\n");
      return;
    }
  char pos[16] = { 0 };
  strncpy (pos, separator + 1, sizeof (pos) - 1);

  if (pos[0] < 'A' || pos[0] > 'J' || pos[1] != '-')
    {
      send_to_client (client, "BAD_REQUEST\n");
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
  ThreadInfo thread_info = *(ThreadInfo *)arg;
  free (arg);

  pthread_mutex_t *mutex = &thread_info.mutex;
  Room *rooms = thread_info.rooms;
  Client base_client = thread_info.client;

  Client *client = NULL; // Will point to actual client in room.
  Room *room = NULL;
  Game *game = NULL;

  // Search for an available room.
  pthread_mutex_lock (mutex);
  for (int i = 0; i < NUMBER_OF_ROOMS; ++i)
    {
      game = &(rooms[i].game);
      if (game->state == WAITING || game->state == AVAILABLE)
        {
          room = &rooms[i];
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
  pthread_mutex_unlock (mutex);

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

  // pthread_mutex_lock (mutex);
  if (game->state == READY_TO_START)
    {
      // Send to each client the START_GAME with their boards
      init_game (&room->game);
      send_start_game (room, PLAYER_A);
      send_start_game (room, PLAYER_B);
      game->state = IN_PROGRESS;
    }
  // pthread_mutex_unlock (mutex);

  // After second client joins.
  while (!is_game_over (get_opposing_board (game)))
    {
      char recv_buffer[BUFSIZ] = { 0 };
      int bytes_read
          = recv (client->sockfd, recv_buffer, sizeof (recv_buffer) - 1, 0);
      if (bytes_read == 0)
        {
          log_event (LOG_INFO, "Client disconnection.");
          break;
        }
      else if (bytes_read == -1)
        {
          log_event (LOG_ERROR, "Failed to recv data.");
          break;
        }
      log_event (LOG_DEBUG, "Received message of length %d from client %c.",
                 bytes_read, client->player);

      handle_message (room, client, recv_buffer);

      pthread_mutex_lock (mutex);
      change_turn (game);
      pthread_mutex_unlock (mutex);
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
