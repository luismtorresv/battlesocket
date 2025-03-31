#include "logger.h"
#include "server.h"

int
main ()
{
  // Initialisation.
  init_logger ();

  // Actual program.
  run_server ();

  // Unloading.
  close_logger ();

  return 0;
}
