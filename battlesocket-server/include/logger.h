#ifndef LOGGER_H
#define LOGGER_H

void init_logger (const char *log_filename);
void log_event (const char *event);
void close_logger ();

#endif
