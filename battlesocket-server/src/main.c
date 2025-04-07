#include "logger.h"
#include "server.h"

int
main (int argc, char **argv)
{
  char *log_filename = "server.log"; // Default log filename.
  if (argc == 2)
    log_filename = argv[1];

  // Initialisation.
  init_logger (log_filename);

  // Actual program.
  Room rooms[NUMBER_OF_ROOMS];
  run_server (rooms);

  // Unloading.
  close_logger ();

  return 0;
}
