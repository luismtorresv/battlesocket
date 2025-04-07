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
