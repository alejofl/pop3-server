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

    for (int i = 0; i < read_bytes; i++) {
        const struct parser_event * event = parser_feed(connection->parser, ptr[i]);
        buffer_read_adv(&connection->buffer_object, 1);
        printf("Evento de tipo %d", event->type);
    }

    return next_state;
}

stm_states stm_authorization_read(struct selector_key * key) {
    printf("ESTOY EN EL AUTHORIZATION STATE\n");
    return read_command(key, AUTHORIZATION, true);
}

void stm_quit_arrival(stm_states state, struct selector_key * key) {
    printf("LLEGUE EN EL QUIT STATE\n");
    selector_unregister_fd(key->s, key->fd);
}

