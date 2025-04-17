/*
 * Implementation partly based on:
 *
 * 1. Michael Tuttle's 2012 blog post "Simple logging in C".
 * URL: https://tuttlem.github.io/2012/12/08/simple-logging-in-c.html
 *
 * 2. Oleg Kutkov's 2019 blog post "Simple logger with STDOUT, Files and syslog
 * support for C projects in Linux".
 * URL:
 * https://olegkutkov.me/2019/03/25/simple-logger-with-stdout-files-and-syslog-support-for-c-projects-in-linux/
 */

#include "logger.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COUNT_OF(x) (sizeof (x) / sizeof ((x)[0]))

static FILE *log_file = NULL;

int set_log_file (const char *filename);
static const char *get_level_string (LogLevel level);

static const char *
get_level_string (LogLevel level)
{
  static const char *level_strings[] = {
    [LOG_DEBUG] = "DEBUG",
    [LOG_INFO] = "INFO",
    [LOG_ERROR] = "ERROR",
    [LOG_FATAL] = "FATAL",
  };

  if ( // Check bounds…
      (level < 0 || level >= COUNT_OF (level_strings))
      // Before dereferencing.
      || !level_strings[level])
    exit (EXIT_FAILURE);

  return level_strings[level];
}

// Sets the `log_file` to write to. Returns 0 if successful and 1 otherwise.
int
set_log_file (const char *filename)
{
  if (filename == NULL)
    return 1;

  log_file = fopen (filename, "a");
  if (log_file == NULL)
    {
      fprintf (stderr, "error: failed to open log file \"%s\": %s\n", filename,
               strerror (errno));
      return 1;
    }

  return 0;
}

// Initialises a logger.
// Exits in case of failure.
void
init_logger (const char *log_filename)
{
  if (set_log_file (log_filename))
    exit (EXIT_FAILURE);
}

// Log a message with a certain `level`.
// This variadic function is meant to be used as e.g. `printf`.
void
log_event (LogLevel level, const char *fmt, ...)
{
  // Get current time.
  time_t now;
  if ((now = time (NULL)) == (time_t)(-1))
    {
      fprintf (stderr, "error: failed to get current time.\n");
      exit (EXIT_FAILURE);
    }

  // Convert it to (almost) ISO 8601 date time format.
  struct tm datetime;
  char datestr[BUFSIZ / 32] = { 0 };
  if (localtime_r (&now, &datetime) == NULL)
    {
      fprintf (stderr, "error: failed to get local time.\n");
      exit (EXIT_FAILURE);
    }
  const char *date_format = "%F %T";
  strftime (datestr, sizeof (datestr) - 1, date_format, &datetime);

  // Write down passed message (this is a variadic function).
  char intermediate_message[BUFSIZ] = { 0 };
  va_list ap;
  va_start (ap, fmt);
  vsnprintf (intermediate_message, sizeof (intermediate_message), fmt, ap);
  va_end (ap);

  // Finally put it all together.
  char final_message[2 * BUFSIZ] = { 0 };
  snprintf (final_message, sizeof (final_message), "%s - [%s] - %s\n", datestr,
            get_level_string (level), intermediate_message);

  // Write to the terminal…
  FILE *stream = (level == LOG_FATAL) ? stderr : stdout;
  fputs (final_message, stream);
  // As well as to the log, if it's defined.
  if (log_file)
    {
      fputs (final_message, log_file);
      fflush (log_file);
    }
}

// Close log file (if defined).
void
close_logger ()
{
  if (log_file != NULL)
    {
      fclose (log_file);
    }
}
