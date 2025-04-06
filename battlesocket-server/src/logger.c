#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATE_FORMAT "%F %T"
#define BUFFER_SIZE 1024

FILE *log_file = NULL;

void
init_logger ()
{
  log_file = fopen ("server.log", "a");
  if (log_file == NULL)
    {
      perror ("Failed to open the log file");
    }
}

void
log_event (const char *event)
{
  time_t now = time (NULL);
  if (now == (time_t)(-1))
    {
      fprintf (stderr, "Failed to get current time.\n");
      exit (EXIT_FAILURE);
    }

  struct tm datetime;
  if (localtime_r (&now, &datetime) == NULL)
    {
      fprintf (stderr, "Failed to get local time.\n");
      exit (EXIT_FAILURE);
    }

  char date_as_text[BUFFER_SIZE] = { 0 };
  strftime (date_as_text, BUFFER_SIZE, DATE_FORMAT, &datetime);

  char log_message[BUFFER_SIZE * 2] = { 0 };
  snprintf (log_message, BUFFER_SIZE * 2, "%s %s\n", date_as_text, event);

  fputs (log_message, stdout);
  if (log_file != NULL)
    {
      fputs (log_message, log_file);
      fflush (log_file);
    }
}

void
close_logger ()
{
  if (log_file != NULL)
    {
      fclose (log_file);
    }
}
