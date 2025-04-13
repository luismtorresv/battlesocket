#include "server.h"

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
