#include "server.h"
#include <argp.h>
#include <signal.h>
#include <stdlib.h>

// Program documentation.
static char doc[] = "battlesocket-server"
                    " -- a concurrent multi-threaded battleship server.";

// The options we can pass.
static struct argp_option options[] = {
  { 0, 'p', "PORT", OPTION_ARG_OPTIONAL,
    "Port for server to listen to (default: 8080)", 0 },
  { 0, 'l', "LOG_FILE", OPTION_ARG_OPTIONAL,
    "Path to write logs to (default: server.log)", 0 },
  { 0 },
};

// To communicate with `parse_opt`.
struct arguments
{
  int port;
  char *log_file;
};

// Parse an option.
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'p':
      if (arg)
        {
          arguments->port = atoi (arg);
        }
      break;
    case 'l':
      arguments->log_file = arg;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

// Arg parser.
static struct argp argp = { options, parse_opt, 0, doc, 0, 0, 0 };

int
main (int argc, char **argv)
{
  // Set seed using current time. Once for all the program.
  srand (time (NULL));

  // We don't want to deal with "broken pipe" errors.
  signal (SIGPIPE, SIG_IGN);

  struct arguments arguments;

  // Default values.
  arguments.port = 8080;
  arguments.log_file = "server.log";

  // Parse arguments and store them in `arguments` struct.
  if (0 != argp_parse (&argp, argc, argv, 0, 0, &arguments))
    {
      perror ("argp_parse");
      exit (EXIT_FAILURE);
    }

  run_server (arguments.port, arguments.log_file);

  return 0;
}
