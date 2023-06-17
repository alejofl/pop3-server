#ifndef LOGGER_H
#define LOGGER_H

#include "selector.h"

#define LOG_DEBUG

int logger_init(fd_selector selector);

int logger_destroy(fd_selector selector);

// The resulting char * must be at most 255 characters long.
void log_debug(const char * const format, ...);

// The resulting char * must be at most 255 characters long.
void log_info(const char * const format, ...);

// The resulting char * must be at most 255 characters long.
void log_error(const char * const format, ...);

#endif
