#include <sys/socket.h>
#include <stdlib.h>

#include <buffer.h>
#include <stm.h>
#include <parser.h>
#include <string.h>

#include "constants.h"
#include "pop3.h"
#include "pop3_stm.h"
#include "pop3_parser.h"

static void handle_read(struct selector_key * key);
static void handle_write(struct selector_key * key);
static void handle_close(struct selector_key * key);

extern struct args args;
extern struct stats stats;

struct state_definition stm_states_table[] = {
        {
                .state = AUTHORIZATION,
                .on_arrival = stm_authorization_arrival,
                .on_departure = stm_authorization_departure,
                .on_read_ready = stm_authorization_read,
                .on_write_ready = stm_authorization_write
        },
        {
                .state = TRANSACTION,
                .on_arrival = stm_transaction_arrival,
                .on_departure = stm_transaction_departure,
                .on_read_ready = stm_transaction_read,
                .on_write_ready = stm_transaction_write
        },
        {
                .state = ERROR,
                .on_arrival = stm_error_arrival,
                .on_departure = stm_error_departure,
                .on_read_ready = stm_error_read,
                .on_write_ready = stm_error_write
        },
        {
                .state = QUIT,
                .on_arrival = stm_quit_arrival,
                .on_departure = stm_quit_departure,
                .on_read_ready = stm_quit_read,
                .on_write_ready = stm_quit_write
        }
};

static const struct parser_state_transition parser_command_state[] = {
        {.when = ' ', .dest = ARGUMENT_1, .act1 = parser_command_state_space},
        {.when = '\r', .dest = END, .act1 = parser_command_state_carriage_return},
        {.when = ANY, .dest = COMMAND, .act1 = parser_command_state_any}
};

static const struct parser_state_transition parser_argument_1_state[] = {
        {.when = '\r', .dest = END, .act1 = parser_argument_1_state_carriage_return},
        {.when = ANY, .dest = ARGUMENT_1, .act1 = parser_argument_1_state_any}
};

static const struct parser_state_transition parser_end_state[] = {
        {.when = '\n', .dest = COMMAND, .act1 = parser_end_state_line_feed},
        {.when = ANY, .dest = COMMAND, .act1 = parser_end_state_any}
};

static const struct parser_state_transition * parser_state_table[] = {
        parser_command_state,
        parser_argument_1_state,
        parser_end_state
};

static const size_t parser_state_n[] = {
        sizeof(parser_command_state) / sizeof(parser_command_state[0]),
        sizeof(parser_argument_1_state) / sizeof(parser_argument_1_state[0]),
        sizeof(parser_end_state) / sizeof(parser_end_state[0]),
};

static const struct parser_definition parser_definition = {
        .states = parser_state_table,
        .states_count = PARSER_STATES_COUNT,
        .start_state = COMMAND,
        .states_n = parser_state_n
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

    buffer_init(&connection->in_buffer_object, BUFFER_SIZE, (uint8_t *) connection->in_buffer);
    buffer_init(&connection->out_buffer_object, BUFFER_SIZE, (uint8_t *) connection->out_buffer);
    connection->parser = parser_init(parser_no_classes(), &parser_definition);
    connection->stm.states = stm_states_table;
    connection->stm.initial = AUTHORIZATION;
    connection->stm.max_state = STM_STATES_COUNT;
    stm_init(&connection->stm);
    connection->last_state = -1;
    connection->current_command.mail_fd = -1;
    buffer_init(&connection->current_command.mail_buffer_object, BUFFER_SIZE, (uint8_t *) connection->current_command.mail_buffer);
    connection->current_session.mails = calloc(args.max_mails, sizeof(struct mail));
    connection->current_session.requested_quit = false;

    if (selector_register(key->s, new_socket_fd, &handler, OP_WRITE, connection) != SELECTOR_SUCCESS) {
        parser_destroy(connection->parser);
        free(connection->current_session.mails);
        free(connection);
        close(new_socket_fd);
        return;
    }
    stats.concurrent_connections++;
    stats.historical_connections++;
}

static void handle_read(struct selector_key * key) {
    stm_handler_read(&((connection_data) key->data)->stm, key);
}

static void handle_write(struct selector_key * key) {
    stm_handler_write(&((connection_data) key->data)->stm, key);

    connection_data connection = (connection_data) key->data;

    size_t read_bytes;
    char * ptr = (char *) buffer_read_ptr(&connection->out_buffer_object, &read_bytes);
    ssize_t n = send(key->fd, ptr, read_bytes, 0);
    buffer_read_adv(&connection->out_buffer_object, n);
    stats.transferred_bytes += n;

    if (connection->current_command.finished) {
        if (connection->current_session.requested_quit) {
            selector_unregister_fd(key->s, key->fd);
        } else if (buffer_can_read(&connection->in_buffer_object)) {
            selector_set_interest_key(key, OP_NOOP);
            stm_handler_read(&((connection_data) key->data)->stm, key);
        } else {
            selector_set_interest_key(key, OP_READ);
        }
    }
}

static void handle_close(struct selector_key * key) {
    connection_data connection = (connection_data) key->data;

    for (int i = 0; i < args.users_count; i++) {
        if (strcmp(args.users[i].name, connection->current_session.username) == 0) {
            args.users[i].logged_in = false;
        }
    }

    parser_destroy(connection->parser);
    free(connection->current_session.mails);
    free(connection);
    close(key->fd);

    stats.concurrent_connections--;
}