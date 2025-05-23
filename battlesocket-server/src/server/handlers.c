#include <poll.h>
#include <time.h>

#include "protocol.h"
#include "server.h"

#define TO_MILLISECONDS(x) (x * 1000)

// Handle a message of the protocol.
void
handle_message (Room *room, Client *client, char *message)
{
  // Remove terminator token.
  char *terminator_pos = strstr (message, TERMINATOR);
  if (!terminator_pos)
    {
      send_bad_request (client);
      return;
    }
  *terminator_pos = '\0';

  log_event (LOG_INFO,
             "Room with id %d : Received \"%s\" from client with IP %s:%ld.",
             room->id, message, inet_ntoa (client->addr.sin_addr),
             client->addr.sin_port);

  Game *game = &room->game;
  pthread_mutex_t *mutex = &room->mutex;

  pthread_mutex_lock (mutex);
  Board *opposing_board = get_opposing_board (game);
  pthread_mutex_unlock (mutex);

  MessageType message_type = parse_message (message);
  if (message_type == MSG_BAD_REQUEST)
    {
      send_bad_request (client);
      return;
    }

  if (message_type == MSG_SHOT)
    {
      Shot shot = parse_shot (message);

      if (!shot.is_valid_shot)
        {
          send_bad_request (client);
          return;
        }

      if (client->player != room->game.current_player)
        return;

      // The shot happens in the board of the opposing player
      bool was_hit = was_ship_hit (opposing_board, shot.row, shot.col);
      pthread_mutex_lock (mutex);
      update_board (opposing_board, shot.row, shot.col, was_hit);
      pthread_mutex_unlock (mutex);

      bool sunk = false;
      if (was_hit)
        {
          Ship *ship = get_ship (opposing_board, shot.row, shot.col);
          if (ship)
            sunk = is_ship_sunk (ship);
        }

      char action_msg[BUFSIZ] = { 0 };
      build_action_result (action_msg, was_hit, shot.row, shot.col, sunk);
      multicast (action_msg, room);

      // Notify both clients that the turn changed if the game is not over.
      pthread_mutex_lock (mutex);
      if (!is_game_over (opposing_board))
        {
          room_change_turn (room);
        }
      pthread_mutex_unlock (mutex);
    }
  else if (message_type == MSG_SURRENDER)
    {
      game->state = FINISHED;
      multicast_end_game (room, client->player, SURRENDER);
    }
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

  room_change_turn (room);

  // Based on "Beej’s Guide to Network Programming", section 7.2
  // "`poll()`---Synchronous I/O Multiplexing":
  //
  // "So how can you avoid polling? Not slightly ironically, you can avoid
  // polling by using the `poll()` system call. In a nutshell, we’re going to
  // ask the operating system to do all the dirty work for us, and just let us
  // know when some data is ready to read on which sockets. In the meantime,
  // our process can go to sleep, saving system resources."
  while (!should_room_finish (room))
    {
      const int CLIENTS_PER_ROOM = 2;

      struct pollfd pfds[CLIENTS_PER_ROOM];
      pfds[0].fd = room->client_a.sockfd;
      pfds[1].fd = room->client_b.sockfd;
      pfds[0].events = pfds[1].events = POLLIN; // Is socket ready to read?

      time_t current_time = get_current_time ();
      time_t real_timeout = room->turn_max_timeval - current_time;
      int num_events
          = poll (pfds, CLIENTS_PER_ROOM, TO_MILLISECONDS (real_timeout));

      if (num_events == 0)
        {
          log_event (LOG_INFO,
                     "Room with id %d : Server didn't get action message"
                     " from player %c"
                     " in %d seconds.",
                     room->id, room->game.current_player, real_timeout);
          game->state = FINISHED;
          multicast_end_game (room, room->game.current_player, TIMEOUT);
          continue;
        }

      int found_events = 0;
      for (int i = 0; i < CLIENTS_PER_ROOM && found_events < num_events; i++)
        {
          bool pollin_happened = pfds[i].revents & POLLIN;

          if (!pollin_happened)
            continue;

          bool is_client_a = i == 0;
          Client *client = is_client_a ? &room->client_a : &room->client_b;

          char recv_buffer[BUFSIZ] = { 0 };
          int bytes_read
              = recv (pfds[i].fd, recv_buffer, sizeof (recv_buffer) - 1, 0);
          if (bytes_read <= 0)
            {
              if (bytes_read == 0)
                {
                  log_event (LOG_INFO, "Client disconnection.");

                  pthread_mutex_lock (mutex);
                  game->state = FINISHED;
                  multicast_end_game (room, client->player, DISCONNECTION);
                  pthread_mutex_unlock (mutex);
                  return NULL;
                }
              else if (bytes_read <= -1)
                {
                  log_event (LOG_ERROR, "Failed to recv data.");
                }
            }
          else
            {
              handle_message (room, client, recv_buffer);
            }
        }
    }

  pthread_mutex_lock (mutex);
  if (is_game_over (get_opposing_board (game)))
    {
      game->state = FINISHED;
      Player winner = game->current_player;
      multicast_end_game (room, winner, WINNER); // Notify who won.
      log_event (LOG_INFO, "Game of room %d is over. Player %c won.", room->id,
                 winner);
    }
  pthread_mutex_unlock (mutex);

  return NULL;
}

// Handle two clients throughout the entire game session.
void *
handle_room (void *arg)
{
  // Copy of the argument.
  ThreadInfo thread_info = *(ThreadInfo *)arg;
  free (arg);

  Client client_a = thread_info.client_a;
  Client client_b = thread_info.client_b;

  Room *room = search_available_room (thread_info.rooms);
  if (room == NULL) // We didn't find a room.
    {
      log_event (LOG_ERROR, "Server is full. Could not find a room.");
      close (client_a.sockfd);
      close (client_b.sockfd);
      return NULL;
    }

  // If we got here, we found a free room.
  Game *game = &room->game;
  pthread_mutex_t *mutex = &room->mutex;

  pthread_mutex_lock (mutex);
  client_a.room = room;
  room->client_a = client_a;
  room->client_a.player = PLAYER_A;

  client_b.room = room;
  room->client_b = client_b;
  room->client_b.player = PLAYER_B;

  game->state = READY_TO_START;

  if (game->state == READY_TO_START)
    {
      pthread_t thread_id;
      pthread_create (&thread_id, NULL, handle_game, room);
      pthread_detach (thread_id);
    }
  pthread_mutex_unlock (mutex);

  return NULL;
}
