#include "logger.h"
#include "server.h"

int
main (int argc, char **argv)
{
  srand (
      time (NULL)); // Set seed using current time, once for all the program.

  char *log_filename = "server.log"; // Default log filename.
  if (argc == 2)
    log_filename = argv[1];

  // Initialisation.
  init_logger (log_filename);

  // Actual program.
  run_server ();

  // Unloading.
  close_logger ();

  return 0;
}
