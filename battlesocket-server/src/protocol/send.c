#include "protocol.h"
#include "server.h"

#define COUNT_OF(x) (sizeof (x) / sizeof ((x)[0]))

// Sends a message to a client.
void
send_to_client (Client *client, const char *message)
{
  send (client->sockfd, message, strlen (message), 0);

  // Log action, but create a copy first.
  char message_copy[4 * BUFSIZ] = { 0 };
  strncpy (message_copy, message, strlen (message));

  // Remove terminator token.
  char *terminator_pos = strstr (message_copy, TERMINATOR);
  if (terminator_pos)
    {
      *terminator_pos = '\0';
    }

  int id = (client->room) ? client->room->id : -1;
  log_event (
      LOG_INFO, "Room with id %d : Sent \"%s\" to client with IP %s:%ld.", id,
      message_copy, inet_ntoa (client->addr.sin_addr), client->addr.sin_port);
}

// Sends a message to both clients of a room.
void
multicast (const char *message, Room *room)
{
  send_to_client (&room->client_a, message);
  send_to_client (&room->client_b, message);
}

void
send_bad_request (Client *client)
{
  char message[BUFSIZ] = { 0 };
  sprintf (message, "BAD_REQUEST" TERMINATOR);
  send_to_client (client, message);
}

// Send a START_GAME message to `client`.
// - `initial_player` is the letter that identifies the player who goes first.
// - `ship_data` is the text representation of the board.
void
send_start_game (Client *client, Player player, const char *ship_data)
{
  char message[BUFSIZ] = { 0 };
  build_start_game (message, player, ship_data);
  send_to_client (client, message);
}

// Send a JOINED_MATCHMAKING message to `client` with their corresponding
// `player` letter.
void
send_joined_matchmaking (Client *client)
{
  char message[BUFSIZ] = { 0 };
  sprintf (message, "JOINED_MATCHMAKING" TERMINATOR);
  send_to_client (client, message);
}

// Send a `TURN` message to both clients.
void
multicast_current_turn (Room *room)
{
  Game *game = &room->game;

  char message[BUFSIZ] = { 0 };
  build_turn_msg (message, game->current_player);
  multicast (message, room);
}

void
multicast_end_game (Room *room, Player player, EndGameReason reason)
{
  // Static string table to map enum to a string.
  static const char *reason_strings[] = {
    [DISCONNECTION] = "DISCONNECTION",
    [WINNER] = "WINNER",
    [SURRENDER] = "SURRENDER",
    [TIMEOUT] = "TIMEOUT",
  };

  if (
      // Bounds check before dereferencing.
      (reason < 0 || reason >= COUNT_OF (reason_strings))
      // Check whether it's a null pointer (i.e. is string above unitialised?)
      || !reason_strings[reason])
    exit (EXIT_FAILURE);

  char message[BUFSIZ] = { 0 };
  const char *reason_string = reason_strings[reason];

  snprintf (message, sizeof (message), "END_GAME %s %c" TERMINATOR,
            reason_string, player);
  multicast (message, room);
}
