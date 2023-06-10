#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"
#include "constants.h"
#include "pop3.h"

static void handle_read(struct selector_key * key);
static void handle_write(struct selector_key * key);
static void handle_close(struct selector_key * key);

void accept_pop_connection(struct selector_key * key) {
    struct sockaddr client_address;
    socklen_t client_address_length = sizeof(client_address);

    int new_socket_fd = accept(key->fd, &client_address, &client_address_length);

    buffer * buffer = calloc(1, sizeof(struct buffer));
    char * buff = calloc(BUFFER_SIZE, 1);

    buffer_init(buffer, BUFFER_SIZE, (uint8_t *) buff);

    const struct fd_handler handler = {
            .handle_read       = handle_read,
            .handle_write      = handle_write,
            .handle_close      = handle_close, // nada que liberar
    };

    selector_register(key->s, new_socket_fd, &handler, OP_READ, buffer);
}

static void handle_read(struct selector_key * key) {
    printf("I'm going to read.\n");
    buffer * buff = (buffer *) key->data;
    size_t write_bytes;
    char * ptr = (char *) buffer_write_ptr(buff, &write_bytes);
    int n = read(key->fd, ptr, write_bytes);
    printf("El n es %d\n", n);
    buffer_write_adv(buff, n);
    printf("Pase el write advanced\n");

    selector_set_interest_key(key, OP_WRITE);
    printf("Pase el interest\n");
}

static void handle_write(struct selector_key * key) {
    printf("I'm going to write.\n");
    buffer * buff = (buffer *) key->data;
    size_t read_bytes;
    char * ptr = (char *) buffer_read_ptr(buff, &read_bytes);
    int n = write(key->fd, ptr, read_bytes);
    buffer_read_adv(buff, n);
    selector_set_interest_key(key, OP_READ);
}

static void handle_close(struct selector_key * key) {
    printf("Closed.\n");
}