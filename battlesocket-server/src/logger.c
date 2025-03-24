#include <stdio.h>
#include <time.h>
#include "logger.h"

FILE *log_file = NULL;

void init_logger() {
    log_file = fopen("server.log", "a");
    if (log_file == NULL) {
        perror("Failed to open the log file");
    }
}

void log_event(const char *event) {
    time_t now = time(NULL);
    fprintf(stdout, "[%ld] %s\n", now, event);
    if (log_file != NULL) {
        fprintf(log_file, "[%ld] %s\n", now, event);
        fflush(log_file);
    }
}

void close_logger() {
    if (log_file != NULL) {
        fclose(log_file);
    }
}
