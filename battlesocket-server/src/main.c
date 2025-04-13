#include "server.h"

int
main (int argc, char **argv)
{
  // Set seed using current time. Once for all the program.
  srand (time (NULL));

  char *log_filename = "server.log"; // Default log filename.
  if (argc == 2)
    log_filename = argv[1];

  run_server (log_filename);

  return 0;
}
