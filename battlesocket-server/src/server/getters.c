#include "server.h"

// Returns true if room is not occupied.
bool
is_room_available (Room *room)
{
  return (room->client_a.sockfd == 0) || (room->client_b.sockfd == 0);
}

// Considers state as well as boards.
bool
should_room_finish (Room *room)
{
  Game *game = &room->game;
  return (game->state == FINISHED) || is_game_over (get_opposing_board (game));
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

// Returns a pointer to the opposing client assigned to the opossing player.
Client *
get_opposing_client (Room *room, Player player)
{
  switch (player)
    {
    case PLAYER_A:
      return &room->client_b;
    case PLAYER_B:
      return &room->client_a;
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
