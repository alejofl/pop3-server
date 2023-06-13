#include <sys/socket.h>
#include <stdio.h>
#include "pop3_stm.h"
#include "pop3.h"
#include "pop3_commands.h"
#include <parser.h>
#include <string.h>

struct pop3_command authorization_commands[] = {
        {.command = "USER", .argument_1_type = REQUIRED, .argument_2_type = EMPTY, .handler = authorization_user},
        {.command = "PASS", .argument_1_type = REQUIRED, .argument_2_type = EMPTY, .handler = authorization_pass},
        {.command = "CAPA", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = authorization_capa},
        {.command = "QUIT", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = authorization_quit},
};

struct pop3_command transaction_commands[] = {
        {.command = "STAT", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = transaction_stat},
        {.command = "LIST", .argument_1_type = OPTIONAL, .argument_2_type = EMPTY, .handler = transaction_list},
        {.command = "RETR", .argument_1_type = REQUIRED, .argument_2_type = EMPTY, .handler = transaction_retr},
        {.command = "DELE", .argument_1_type = REQUIRED, .argument_2_type = EMPTY, .handler = transaction_dele},
        {.command = "NOOP", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = transaction_noop},
        {.command = "RSET", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = transaction_rset},
        {.command = "TOP", .argument_1_type = REQUIRED, .argument_2_type = REQUIRED, .handler = transaction_top},
        {.command = "CAPA", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = transaction_capa},
        {.command = "QUIT", .argument_1_type = REQUIRED, .argument_2_type = REQUIRED, .handler = transaction_quit},
};

struct pop3_command * pop3_commands[] = {
    authorization_commands,
    transaction_commands
};

size_t pop3_commands_length[] = {
        sizeof(authorization_commands) / sizeof(authorization_commands[0]),
        sizeof(transaction_commands) / sizeof(transaction_commands[0])
};

stm_states read_command(struct selector_key * key, stm_states next_state, bool read_from_socket) {
    connection_data connection = (connection_data) key->data;

    // TODO we may need to use de read_from_socket flag.

    size_t write_bytes;
    char * ptr = (char *) buffer_write_ptr(&connection->buffer_object, &write_bytes);
    ssize_t n = recv(key->fd, ptr, write_bytes, 0);

    buffer_write_adv(&connection->buffer_object, n);

    size_t read_bytes;
    ptr = (char *) buffer_read_ptr(&connection->buffer_object, &read_bytes);

    connection->current_command.command[0] = '\0';
    connection->current_command.command_length = 0;
    connection->current_command.argument_1[0] = '\0';
    connection->current_command.argument_1_length = 0;
    connection->current_command.argument_2[0] = '\0';
    connection->current_command.argument_2_length = 0;

    for (int i = 0; i < read_bytes; i++) {
        const struct parser_event * event = parser_feed(connection->parser, ptr[i], connection);
        buffer_read_adv(&connection->buffer_object, 1);

        if (event->type == VALID_COMMAND) {
            printf("Command: %s\nArgument 1: %s\nArgument 2: %s\n", connection->current_command.command, connection->current_command.argument_1, connection->current_command.argument_2);
            for (int j = 0; j < pop3_commands_length[next_state]; j++) {
                struct pop3_command maybe_command = pop3_commands[next_state][j];
                if (strcmp(maybe_command.command, connection->current_command.command) == 0) {
                    if ((maybe_command.argument_1_type == REQUIRED && connection->current_command.argument_1_length > 0) ||
                        (maybe_command.argument_1_type == EMPTY && connection->current_command.argument_1_length == 0) ||
                        (maybe_command.argument_1_type == OPTIONAL)) {
                        if ((maybe_command.argument_2_type == REQUIRED && connection->current_command.argument_2_length > 0) ||
                            (maybe_command.argument_2_type == EMPTY && connection->current_command.argument_2_length == 0) ||
                            (maybe_command.argument_2_type == OPTIONAL)) {
                            stm_states sig_estado = maybe_command.handler(connection);
                            printf("El siguiente estado es %d\n", sig_estado);
                            return sig_estado;
                        } else {
                            printf("ERROR EN ARGUMENT 2\n");
                            return ERROR;
                        }
                    } else {
                        printf("ERROR EN ARGUMENT 1\n");
                        return ERROR;
                    }
                }
            }
            printf("ERROR COMANDO NO VALIDO PARA EL ESTADO\n");
            return ERROR;
        } else if (event->type == INVALID_COMMAND) {
            printf("ERROR INVALID COMMAND PERO SINTACTICAMENTE\n");
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
    printf("ESTOY EN EL TRANSACTION STATE\n");
    return read_command(key, TRANSACTION, true);
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
