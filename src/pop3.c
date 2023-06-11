#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"
#include "constants.h"
#include "pop3.h"

static void handle_read(struct selector_key * key);
static void handle_write(struct selector_key * key);
static void handle_close(struct selector_key * key);

const struct fd_handler handler = {
        .handle_read  = handle_read,
        .handle_write = handle_write,
        .handle_close = handle_close,
};

void accept_pop_connection(struct selector_key * key) {
    struct sockaddr client_address;
    socklen_t client_address_length = sizeof(client_address);

    int new_socket_fd = accept(key->fd, &client_address, &client_address_length);

    buffer * buffer = calloc(1, sizeof(struct buffer));
    char * buff = calloc(BUFFER_SIZE, 1);

    buffer_init(buffer, BUFFER_SIZE, (uint8_t *) buff);

    selector_register(key->s, new_socket_fd, &handler, OP_READ, buffer);
}

static void handle_read(struct selector_key * key) {
    buffer * buff = (buffer *) key->data;
    size_t write_bytes;
    char * ptr = (char *) buffer_write_ptr(buff, &write_bytes);
    ssize_t n = recv(key->fd, ptr, write_bytes, 0);
    if (n <= 0) {
        selector_unregister_fd(key->s, key->fd);
        return;
    }
    buffer_write_adv(buff, n);
    selector_set_interest_key(key, OP_WRITE);
}

static void handle_write(struct selector_key * key) {
    printf("feirojfierjgir\n");
    buffer * buff = (buffer *) key->data;
    size_t read_bytes;
    char * ptr = (char *) buffer_read_ptr(buff, &read_bytes);
    ssize_t n = send(key->fd, ptr, read_bytes, 0);
    if (n == -1) {
        selector_unregister_fd(key->s, key->fd);
        return;
    }
    buffer_read_adv(buff, n);
    selector_set_interest_key(key, OP_READ);
}

static void handle_close(struct selector_key * key) {
    buffer * buff = (buffer *) key->data;
    free(buff->data);
    free(buff);
}