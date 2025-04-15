#include "protocol.h"
#include "server.h"

#include <sys/socket.h>
#include <sys/time.h>

void
room_change_turn (Room *room)
{
  const int TURN_TIME_SECS = 30;

  change_turn (&room->game);

  room->turn_max_timeval = get_current_time () + TURN_TIME_SECS;

  multicast_current_turn (room);
}

time_t
get_current_time ()
{
  time_t current_time = time (NULL);
  if (current_time == (time_t)(-1))
    {
      log_event (LOG_ERROR, "Failed to get current time.");
      exit (EXIT_FAILURE);
    }
  return current_time;
}

Room *
search_available_room (Room *rooms)
{
  Room *room = NULL;
  for (int i = 0; i < NUMBER_OF_ROOMS; ++i)
    {
      Game *game = &(rooms[i].game);

      pthread_mutex_lock (&rooms[i].mutex);
      if (game->state == WAITING || game->state == AVAILABLE)
        {
          room = &rooms[i];
          pthread_mutex_unlock (&rooms[i].mutex);
          return room; // Found it!
        }
      pthread_mutex_unlock (&rooms[i].mutex);
    }
  return room; // Didn't find a room.
}

// From "Beej's Guide to Network Programming", Chapter 8 "Common Questions",
// "How can I implement a timeout on a call to recv()?"
int
recvtimeout (int sockfd, char *buf, int buflen, int timeout)
{
  fd_set fds;
  int n;
  struct timeval tv;

  // Set up file descriptor.
  FD_ZERO (&fds);
  FD_SET (sockfd, &fds);

  // Set up timer.
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  // Wait until timeout or data.
  n = select (sockfd + 1, &fds, NULL, NULL, &tv);
  if (n == 0)
    return -2; // Timeout.
  if (n == -1)
    return -1; // Error.

  // If we received data.
  return recv (sockfd, buf, buflen, 0);
}

bool
handshake (int client_fd)
{
  char recv_buffer[BUFSIZ] = { 0 };
  const int timeout = 5; // In seconds.

  int n = recvtimeout (client_fd, recv_buffer, sizeof (recv_buffer), timeout);
  if (n < 0)
    return false; // Didn't get handshake.

  const char *expected_request = "JOIN" TERMINATOR;
  if (strncmp (recv_buffer, expected_request, sizeof (*expected_request)) == 0)
    return true;
  return false; // Not the message we expected.
}
