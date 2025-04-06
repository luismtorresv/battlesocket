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

static FILE *log_file = NULL;

int set_log_file (const char *filename);
void __log (LogLevel level, const char *fmt, va_list ap);
void __logv (LogLevel level, const char *fmt, ...);

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

void
init_logger (const char *log_filename)
{
  if (set_log_file (log_filename))
    exit (EXIT_FAILURE);
}

void
__log (LogLevel level, const char *fmt, va_list ap)
{
  va_list save; // Copy to be used in log file.
  FILE *stream; // We want to print to the terminal.

  va_copy (save, ap);
  stream = (level == LOG_FATAL) ? stderr : stdout;
  vfprintf (stream, fmt, ap); // ap gets exhausted here.
  if (log_file != NULL)
    {
      vfprintf (log_file, fmt, save);
      fflush (log_file);
    }
}

void
__logv (LogLevel level, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  __log (level, fmt, ap);
  va_end (ap);
}

void
log_event (LogLevel level, const char *fmt, ...)
{
  const char *DATE_FORMAT = "%F %T";

  va_list ap;
  time_t now;
  struct tm datetime;
  char datestr[BUFSIZ] = { 0 };

  if ((now = time (NULL)) == (time_t)(-1))
    {
      fprintf (stderr, "error: failed to get current time.\n");
      exit (EXIT_FAILURE);
    }
  if (localtime_r (&now, &datetime) == NULL)
    {
      fprintf (stderr, "error: failed to get local time.\n");
      exit (EXIT_FAILURE);
    }
  strftime (datestr, sizeof (datestr) - 1, DATE_FORMAT, &datetime);
  __logv (level, "%s ", datestr);

  const char *LEVEL_FORMAT = "[%s] ";
  switch (level)
    {
    case LOG_DEBUG:
      __logv (level, LEVEL_FORMAT, "DEBUG");
      break;
    case LOG_INFO:
      __logv (level, LEVEL_FORMAT, "INFO");
      break;
    case LOG_ERROR:
      __logv (level, LEVEL_FORMAT, "ERROR");
      break;
    case LOG_FATAL:
      __logv (level, LEVEL_FORMAT, "FATAL");
      break;
    default:
      break;
    }

  // HACK: This specific call already has a variadic list.
  // If we were to call the other function, it would overwrite the current
  // list, which would result in garbage output.
  va_start (ap, fmt);
  __log (level, fmt, ap);
  va_end (ap);

  __logv (level, "\n");
}

void
close_logger ()
{
  if (log_file != NULL)
    {
      fclose (log_file);
    }
}
