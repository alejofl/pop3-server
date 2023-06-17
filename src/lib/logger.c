#include "include/logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include "include/selector.h"
#include "include/buffer.h"

#define LOGFILE "turtle-pop3.log"
#define PERMISSION_BITS 0644
#define LOG_BUFFER_SIZE 4096
#define MAX_LOG_MESSAGE_LENGTH 255
#define MAX_LOG_TIME_LENGTH 16
#define MAX_LOG_LINE_LENGTH (MAX_LOG_TIME_LENGTH + 1 + 7 + 1 + MAX_LOG_MESSAGE_LENGTH)

void write_to_logfile(struct selector_key * key);
void close_logfile(struct selector_key * key);

struct logger_data {
    int fd;
    struct buffer buffer_object;
    char buffer[LOG_BUFFER_SIZE];
};

static struct logger_data * data;
static fd_selector selec;

static struct fd_handler logger_handler = {
        .handle_write = write_to_logfile,
        .handle_close = close_logfile
};

size_t get_timestamp(char * buffer, size_t buffer_size) {
    time_t t = time(NULL);
    struct tm * tm = localtime(&t);

    return strftime(buffer, buffer_size, "%Y-%m-%dT%H:%M", tm);
}

void write_to_logfile(struct selector_key * key) {
    size_t read_bytes;
    char * ptr = (char *) buffer_read_ptr(&data->buffer_object, &read_bytes);
    ssize_t n = write(data->fd, ptr, read_bytes);
    buffer_read_adv(&data->buffer_object, n);
    selector_set_interest(selec, data->fd, OP_NOOP);
}

void close_logfile(struct selector_key * key) {
    close(key->fd);
}

void do_log(const char * prefix, const char * const format, va_list arguments) {
    size_t write_bytes;
    char * ptr = (char *) buffer_write_ptr(&data->buffer_object, &write_bytes);
    if (MAX_LOG_LINE_LENGTH > write_bytes) {
        return;
    }

    char message[MAX_LOG_MESSAGE_LENGTH + 1];
    vsprintf(message, format, arguments);
    char time[MAX_LOG_TIME_LENGTH + 1];
    get_timestamp(time, MAX_LOG_TIME_LENGTH + 1);

    sprintf(ptr, "%s %s %s\n", time, prefix, message);
    buffer_write_adv(&data->buffer_object, strlen(time) + 1 + strlen(prefix) + 1 + strlen(message) + 1);
    selector_set_interest(selec, data->fd, OP_WRITE);
}

int logger_init(fd_selector selector) {
    if (data != NULL) {
        return 0;
    }

    data = calloc(1, sizeof(struct logger_data));
    buffer_init(&data->buffer_object, LOG_BUFFER_SIZE, (uint8_t *) data->buffer);
    selec = selector;

    data->fd = open(LOGFILE, O_WRONLY | O_APPEND | O_CREAT | O_NONBLOCK, PERMISSION_BITS);
    if (data->fd < 0) {
        return -1;
    }

    if (selector_register(selector, data->fd, &logger_handler, OP_NOOP, data) != SELECTOR_SUCCESS) {
        close(data->fd);
        return -1;
    }
    return 0;
}

int logger_destroy(fd_selector selector) {
    if (data == NULL) {
        return 0;
    }

    selector_unregister_fd(selector, data->fd);
    free(data);
    data = NULL;
    selec = NULL;
    return 0;
}

void log_debug(const char * const format, ...) {
#ifdef LOG_DEBUG
    va_list arguments;
	va_start(arguments, format);
	do_log("[DEBUG]", format, arguments);
	va_end(arguments);
#endif // LOG_DEBUG
}

void log_info(const char * const format, ...) {
    va_list arguments;
    va_start(arguments, format);
    do_log("[INFO ]", format, arguments);
    va_end(arguments);
}

void log_error(const char * const format, ...) {
    va_list arguments;
    va_start(arguments, format);
    do_log("[ERROR]", format, arguments);
    va_end(arguments);
}
