#include "protocol.h"
#include "server.h"

// Handle a message of the protocol.
void
handle_message (Room *room, Client *client, char *message)
{
  Game *game = &room->game;
  pthread_mutex_t *mutex = &room->mutex;

  pthread_mutex_lock (mutex);
  Board *opposing_board = get_opposing_board (game);
  Player opposing_player = game->current_player == PLAYER_A ? PLAYER_B : PLAYER_A;
  pthread_mutex_unlock (mutex);

  int newline_pos = strcspn (message, "\r\n");
  message[newline_pos] = '\0';

  char action[16] = { 0 };
  char pos_str[16] = { 0 };
  if (sscanf (message, "%15s %15s", action, pos_str) != 2)
    {
      send_bad_request (client);

      // Notify both clients that the turn changed.
      char turn_msg[BUFSIZ] = {0};
      long turn_time = time(NULL) + 30;
      build_turn_msg (turn_msg, opposing_player, turn_time);
      broadcast (turn_msg, room);
      return;
    }
  if (strcmp (action, "SHOT") != 0)
    {
      send_bad_request (client);

      // Notify both clients that the turn changed.
      char turn_msg[BUFSIZ] = {0};
      long turn_time = time(NULL) + 30;
      build_turn_msg (turn_msg, opposing_player, turn_time);
      broadcast (turn_msg, room);
      return;
    }

  char row_char;
  int col_val;
  int consumed = 0; // This variable will keep the characters consumed.
                    // We use %n to obtain the number of characters read while
                    // parsing. If there are characters remaining after the
                    // `col_val`, it is a bad request.

  if (sscanf (pos_str, " %c%d%n", &row_char, &col_val, &consumed) != 2
      || pos_str[consumed] != '\0')
    {
      send_bad_request (client);

      // Notify both clients that the turn changed.
      char turn_msg[BUFSIZ] = {0};
      long turn_time = time(NULL) + 30;
      build_turn_msg (turn_msg, opposing_player, turn_time);
      broadcast (turn_msg, room);
      return;
    }
  if (row_char < 'A' || row_char > 'J' || col_val < 1 || col_val > BOARD_SIZE)
    {
      send_bad_request (client);

      // Notify both clients that the turn changed.
      char turn_msg[BUFSIZ] = {0};
      long turn_time = time(NULL) + 30;
      build_turn_msg (turn_msg, opposing_player, turn_time);
      broadcast (turn_msg, room);
      return;
    }

  int row = row_char - 'A';
  int col = col_val - 1;

  log_event (LOG_INFO, "Processing shot of client with IP %s:%ld.",
             inet_ntoa (client->addr.sin_addr), client->addr.sin_port);

  // The shot happens in the board of the opposing player
  bool was_hit = was_ship_hit (opposing_board, row, col);
  pthread_mutex_lock (mutex);
  update_board (opposing_board, row, col, was_hit);
  pthread_mutex_unlock (mutex);

  bool sunk = false;
  if (was_hit)
    {
      int ship_index = get_ship_index_at (opposing_board, row, col);
      if (ship_index != -1)
        sunk = is_ship_sunk (opposing_board, ship_index);
    }

  char action_msg[BUFSIZ] = { 0 };
  build_action_result (action_msg, result, pos_str, sunk);
  broadcast (action_msg, room);

  // Notify both clients that the turn changed if the game is not over.
  if (!is_game_over (opposing_board))
    {
      char turn_msg[BUFSIZ] = {0};
      long turn_time = time(NULL) + 30;
      build_turn_msg (turn_msg, opposing_player, turn_time);
      broadcast (turn_msg, room);
      return;
    }

  log_event (LOG_INFO, "Action message sent");
}

// Send start game message to `player`.
void
notify_start_game (Room *room, Player player)
{
  Board *board = get_board (&room->game, player);
  Client *client = get_client (room, player);

  char ship_data[BUFSIZ] = { 0 };
  get_ship_data (board, ship_data, sizeof (ship_data));

  send_start_game (client, player, ship_data);
}

// Handler for the game.
void *
handle_game (void *arg)
{
  Room *room = (Room *)arg;
  Game *game = &room->game;
  pthread_mutex_t *mutex = &room->mutex;

  pthread_mutex_lock (mutex);
  init_game (&room->game);
  pthread_mutex_unlock (mutex);

  notify_start_game (room, PLAYER_A);
  notify_start_game (room, PLAYER_B);
  char turn_msg[BUFSIZ] = {0};
  long turn_time = time(NULL) + 30;
  build_turn_msg (turn_msg, game->current_player, turn_time);
  broadcast (turn_msg, room);
  pthread_mutex_lock (mutex);
  game->state = IN_PROGRESS;
  pthread_mutex_unlock (mutex);

  char recv_buffer[BUFSIZ] = { 0 };
  while (!is_game_over (get_opposing_board (game)))
    {
      memset (recv_buffer, 0, sizeof (recv_buffer));
      int bytes_read = recv (get_current_socket_fd (room), recv_buffer,
                             sizeof (recv_buffer) - 1, 0);
      if (bytes_read == 0)
        {
          log_event (LOG_INFO, "Client disconnection.");
          broadcast ("END_GAME$", room);
          break;
        }
      else if (bytes_read <= 1)
        {
          log_event (LOG_ERROR, "Failed to recv data.");
          break;
        }
      int newline_pos = strcspn (recv_buffer, "\r\n");
      recv_buffer[newline_pos] = '\0';

      log_event (LOG_DEBUG, "Player message received");
      handle_message (room, get_current_client (room), recv_buffer);
      if (is_game_over (get_opposing_board (game)))
        break;

      pthread_mutex_lock (mutex);
      change_turn (game);
      pthread_mutex_unlock (mutex);
    }

  pthread_mutex_lock (mutex);
  if (is_game_over (get_opposing_board (game)))
    {
      char end_msg[BUFSIZ] = { 0 };
      Player winner = game->current_player;
      build_end_game (end_msg, winner);
      broadcast (end_msg, room);
      game->state = FINISHED;
      log_event (LOG_INFO, "Game over");
    }
  pthread_mutex_unlock (mutex);

  return NULL;
}

// Handle a client throughout the entire game session.
void *
handle_client (void *arg)
{
  // Copy of the argument.
  ThreadInfo thread_info = *(ThreadInfo *)arg;
  free (arg);

  Room *rooms = thread_info.rooms;
  Client base_client = thread_info.client;
  Room *room = NULL;

  // Search for an available room.
  for (int i = 0; i < NUMBER_OF_ROOMS; ++i)
    {
      Game *game = &(rooms[i].game);
      pthread_mutex_lock (&rooms[i].mutex);
      if (game->state == WAITING || game->state == AVAILABLE)
        {
          room = &rooms[i];
          pthread_mutex_unlock (&rooms[i].mutex);
          break;
        }
      pthread_mutex_unlock (&rooms[i].mutex);
    }

  if (room == NULL) // We didn't find a room.
    {
      log_event (LOG_ERROR, "Server is full.");
      close (base_client.sockfd);
      return NULL;
    }

  // If we got here, we found a game.
  Game *game = &room->game;
  pthread_mutex_t *mutex = &room->mutex;
  Client *client = NULL; // Will point to actual client in room.

  // Is first client unassigned?
  pthread_mutex_lock (mutex);
  if (room->client_a.sockfd == 0)
    {
      room->client_a = base_client;
      room->client_a.player = PLAYER_A;
      client = &room->client_a;
      game->state = WAITING;
    }
  else
    {
      // Assign to second client in room.
      room->client_b = base_client;
      room->client_b.player = PLAYER_B;
      client = &room->client_b;
      game->state = READY_TO_START;
    }
  pthread_mutex_unlock (mutex);

  // Send JOINED_MATCHMAKING
  log_event (LOG_INFO, "New client connected");
  send_joined_matchmaking (client);

  while (game->state == WAITING)
    {
      sleep (1);
    }

  // If the room is READY_TO_START, create the thread to manage the game.
  pthread_mutex_lock (mutex);
  if (game->state == READY_TO_START)
    {
      pthread_t thread_id;
      pthread_create (&thread_id, NULL, handle_game, room);
      pthread_detach (thread_id);
    }
  pthread_mutex_unlock (mutex);

  return NULL;
}
