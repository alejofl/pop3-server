#include <sys/socket.h>
#include <stdlib.h>

#include <buffer.h>
#include <stm.h>
#include <parser.h>
#include <stdio.h>

#include "constants.h"
#include "pop3.h"
#include "pop3_stm.h"

static void handle_read(struct selector_key * key);
static void handle_write(struct selector_key * key);
static void handle_close(struct selector_key * key);

struct state_definition stm_states_table[] = {
        {
                .state = AUTHORIZATION,
                .on_arrival = NULL,
                .on_departure = NULL,
                .on_read_ready = stm_authorization_read,
                .on_write_ready = NULL
        },
        {
                .state = TRANSACTION,
                .on_arrival = NULL,
                .on_departure = NULL,
                .on_read_ready = NULL,
                .on_write_ready = NULL
        },
        {
                .state = UPDATE,
                .on_arrival = NULL,
                .on_departure = NULL,
                .on_read_ready = NULL,
                .on_write_ready = NULL
        },
        {
                .state = ERROR,
                .on_arrival = NULL,
                .on_departure = NULL,
                .on_read_ready = NULL,
                .on_write_ready = NULL
        },
        {
                .state = QUIT,
                .on_arrival = stm_quit_arrival,
                .on_departure = NULL,
                .on_read_ready = NULL,
                .on_write_ready = NULL
        }
};

const struct fd_handler handler = {
        .handle_read  = handle_read,
        .handle_write = handle_write,
        .handle_close = handle_close,
};

void accept_pop_connection(struct selector_key * key) {
    struct sockaddr_storage client_address;
    socklen_t client_address_length = sizeof(client_address);

    int new_socket_fd = accept(key->fd, (struct sockaddr *) &client_address, &client_address_length);

    connection_data connection = calloc(1, sizeof(struct connection_data));

    buffer_init(&connection->buffer_object, BUFFER_SIZE, (uint8_t *) connection->buffer);
    connection->parser = parser_init(parser_no_classes(), &parser_definition);
    connection->stm.states = stm_states_table;
    connection->stm.initial = AUTHORIZATION;
    connection->stm.max_state = STM_STATES_COUNT;
    stm_init(&connection->stm);

    if (selector_register(key->s, new_socket_fd, &handler, OP_READ, connection) != SELECTOR_SUCCESS) {
        parser_destroy(connection->parser);
        free(connection);
    }
}

static void handle_read(struct selector_key * key) {
    connection_data connection = (connection_data) key->data;
    struct state_machine stm = connection->stm;
    stm_handler_read(&stm, key);
}

static void handle_write(struct selector_key * key) {
    connection_data connection = (connection_data) key->data;
    size_t read_bytes;
    char * ptr = (char *) buffer_read_ptr(&connection->buffer_object, &read_bytes);
    ssize_t n = send(key->fd, ptr, read_bytes, 0);
    if (n == -1) {
        selector_unregister_fd(key->s, key->fd);
        return;
    }
    buffer_read_adv(&connection->buffer_object, n);
    selector_set_interest_key(key, OP_READ);
}

static void handle_close(struct selector_key * key) {
    connection_data connection = (connection_data) key->data;
    parser_destroy(connection->parser);
    free(connection);
}