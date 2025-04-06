#include "logger.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static FILE *log_file = NULL;

int set_log_file (const char *filename);

int
set_log_file (const char *filename)
{
  if (filename == NULL)
    return 1;

  log_file = fopen (filename, "wa");
  if (log_file == NULL)
    {
      fprintf (stderr, "error: failed to open log file \"%s\": %s\n", filename,
               strerror (errno));
      return 1;
    }

  return 0;
}

void
init_logger (const char *log_filename)
{
  if (set_log_file (log_filename))
    exit (EXIT_FAILURE);
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

  char date_as_text[BUFSIZ] = { 0 };
  const char *DATE_FORMAT = "%F %T";
  strftime (date_as_text, sizeof (date_as_text) - 1, DATE_FORMAT, &datetime);

  char log_message[BUFSIZ * 2] = { 0 };
  snprintf (log_message, sizeof (log_message), "%s %s\n", date_as_text, event);

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
