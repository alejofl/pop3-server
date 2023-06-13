#include <sys/socket.h>
#include <stdio.h>
#include "pop3_stm.h"
#include "pop3.h"

stm_states read_command(struct selector_key * key, stm_states next_state, bool read_from_socket) {
    connection_data connection = (connection_data) key->data;

    // TODO we may need to use de read_from_socket flag.

    size_t write_bytes;
    char * ptr = (char *) buffer_write_ptr(&connection->buffer_object, &write_bytes);
    ssize_t n = recv(key->fd, ptr, write_bytes, 0);

    buffer_write_adv(&connection->buffer_object, n);

    size_t read_bytes;
    ptr = (char *) buffer_read_ptr(&connection->buffer_object, &read_bytes);

    connection->current_command.command_length = 0;
    connection->current_command.argument_1_length = 0;
    connection->current_command.argument_2_length = 0;
    for (int i = 0; i < read_bytes; i++) {
        const struct parser_event * event = parser_feed(connection->parser, ptr[i], connection);
        buffer_read_adv(&connection->buffer_object, 1);

        printf("Evento de tipo %d", event->type);

        if (event->type == VALID_COMMAND) {

        } else if (event->type == INVALID_COMMAND) {
            return ERROR;
        }
    }

    return next_state;
}

void stm_authorization_arrival(stm_states state, struct selector_key * key) {

}
void stm_authorization_departure(stm_states state, struct selector_key * key) {

}
stm_states stm_authorization_read(struct selector_key * key) {
    printf("ESTOY EN EL AUTHORIZATION STATE\n");
    return read_command(key, AUTHORIZATION, true);
}
stm_states stm_authorization_write(struct selector_key * key) {

}

void stm_transaction_arrival(stm_states state, struct selector_key * key) {

}
void stm_transaction_departure(stm_states state, struct selector_key * key) {

}
stm_states stm_transaction_read(struct selector_key * key) {

}
stm_states stm_transaction_write(struct selector_key * key) {

}

void stm_update_arrival(stm_states state, struct selector_key * key) {

}
void stm_update_departure(stm_states state, struct selector_key * key) {

}
stm_states stm_update_read(struct selector_key * key) {

}
stm_states stm_update_write(struct selector_key * key) {

}

void stm_error_arrival(stm_states state, struct selector_key * key) {

}
void stm_error_departure(stm_states state, struct selector_key * key) {

}
stm_states stm_error_read(struct selector_key * key) {

}
stm_states stm_error_write(struct selector_key * key) {

}

void stm_quit_arrival(stm_states state, struct selector_key * key) {
    printf("LLEGUE EN EL QUIT STATE\n");
    selector_unregister_fd(key->s, key->fd);
}
void stm_quit_departure(stm_states state, struct selector_key * key) {

}
stm_states stm_quit_read(struct selector_key * key) {

}
stm_states stm_quit_write(struct selector_key * key) {

}
