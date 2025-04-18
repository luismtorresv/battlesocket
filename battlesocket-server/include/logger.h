#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

// Log level.
// Idea and values for this enum come from raylib.
// https://github.com/raysan5/raylib/blob/ceb1a5ea2b7550aa01c103b4b3dac0dbc4d8f8fe/src/raylib.h#L560-L571
typedef enum
{
  LOG_DEBUG,
  LOG_INFO, // Program execution info
  LOG_ERROR,
  LOG_FATAL, // Used to abort program: exit(EXIT_FAILURE)
} LogLevel;

bool init_logger (const char *log_filename);
void close_logger ();

void log_event (LogLevel level, const char *fmt, ...);
#endif
