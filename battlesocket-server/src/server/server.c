#include "server.h"

typedef struct ServerInfo ServerInfo;
struct ServerInfo
{
  int fd;
  Room *rooms;
};

// Local function declarations.
ServerInfo init_server (Room *rooms);
void cleanup_server (int server_fd);

// Initialise server with its socket.
ServerInfo
init_server (Room *rooms)
{
  ServerInfo server;

  if ((server.fd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
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
  if (setsockopt (server.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse))
      == -1)
    {
      log_event (LOG_ERROR, "Failed to set SO_REUSEADDR.");
      exit (1);
    }

  int keepalive = 1;
  if (setsockopt (server.fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive,
                  sizeof (reuse))
      == -1)
    {
      log_event (LOG_ERROR, "Failed to set SO_KEEPALIVE.");
      exit (1);
    }

  if (bind (server.fd, (struct sockaddr *)&serv_addr, sizeof (serv_addr))
      == -1)
    {
      log_event (LOG_ERROR, "Failed to bind.");
      exit (1);
    }

  if (listen (server.fd, MAX_CLIENTS) == -1)
    {
      log_event (LOG_ERROR, "Failed to listen.");
      exit (1);
    }

  log_event (LOG_INFO, "Server listening with file descriptor %d.", server.fd);

  if (!rooms)
    {
      log_event (LOG_FATAL, "Null pointer to rooms.");
      cleanup_server (server.fd);
    }
  server.rooms = rooms;

  for (int i = 0; i < NUMBER_OF_ROOMS; ++i)
    {
      server.rooms[i].id = i;
      if (pthread_mutex_init (&server.rooms[i].mutex, NULL) != 0)
        {
          log_event (LOG_FATAL, "Failed to initialise mutex for room %d.", i);
          cleanup_server (server.fd);
          exit (1);
        }
    }

  return server;
}

// Accept incoming client connections and dispatch them.
void
run_server ()
{
  Room rooms[NUMBER_OF_ROOMS];
  ServerInfo server = init_server (rooms);

  int client_socket;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len;
  client_addr_len = sizeof (client_addr);

  while ((client_socket = accept (server.fd, (struct sockaddr *)&client_addr,
                                  &client_addr_len))
         != -1)
    {
      pthread_t thread_id;

      // Set up the argument to the thread.
      ThreadInfo thread_info;
      thread_info.client.addr = client_addr;
      thread_info.client.sockfd = client_socket;
      thread_info.rooms = server.rooms;

      ThreadInfo *new_thread_info = malloc (sizeof (ThreadInfo));
      *new_thread_info = thread_info;

      // Create the actual goddamn thread.
      if (pthread_create (&thread_id, NULL, handle_client,
                          (void *)new_thread_info))
        {
          log_event (LOG_ERROR, "Failed to create thread.");
          free (new_thread_info);
        }
      pthread_detach (thread_id);
    }

  cleanup_server (server.fd);
}

// Close socket file descriptors.
void
cleanup_server (int server_fd)
{
  close (server_fd);
  log_event (LOG_INFO, "Server closed.");
  exit (EXIT_SUCCESS);
  // TODO: Close all sockets of all rooms.
}
