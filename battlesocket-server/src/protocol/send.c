#include "protocol.h"
#include "server.h"

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

void
send_bad_request (Client *client)
{
  char message[BUFSIZ] = { 0 };
  sprintf (message, "BAD_REQUEST$");
  send_to_client (client, message);
}

// Send a START_GAME message to `client`.
// - `unix_time` is the game's start time.
// - `initial_player` is the letter that identifies the player who goes first.
// - `ship_data` is the text representation of the board.
void
send_start_game (Client *client, Player player, const char *ship_data)
{
  char message[BUFSIZ] = { 0 };
  build_start_game (message, player, ship_data);
  send_to_client (client, message);
}

// Send an END_GAME message to `client` with the letter of the `winner`.
// - `winner` is the letter that identifies the player who won.
// TODO: Don't use this for notifying of disconnections.
void
send_end_game (Client *client, Player winner)
{
  char message[BUFSIZ] = { 0 };
  build_end_game (message, winner);
  send_to_client (client, message);
}

// Send a JOINED_MATCHMAKING message to `client` with their corresponding
// `player` letter.
void
send_joined_matchmaking (Client *client)
{
  char message[BUFSIZ] = { 0 };
  sprintf (message, "JOINED_MATCHMAKING$");
  send_to_client (client, message);
}
