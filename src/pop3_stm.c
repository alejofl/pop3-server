#include <sys/socket.h>
#include <stdio.h>
#include "pop3_stm.h"
#include "pop3.h"
#include "pop3_commands.h"
#include <parser.h>
#include <string.h>
#include "pop3_parser.h"

struct pop3_command authorization_commands[] = {
        {.command = "USER", .argument_1_type = REQUIRED, .argument_2_type = EMPTY, .handler = authorization_user, .writer = write_authorization_user},
        {.command = "PASS", .argument_1_type = REQUIRED, .argument_2_type = EMPTY, .handler = authorization_pass, .writer = write_authorization_pass},
        {.command = "CAPA", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = authorization_capa, .writer = write_authorization_capa},
        {.command = "QUIT", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = authorization_quit, .writer = write_authorization_quit},
};

struct pop3_command transaction_commands[] = {
        {.command = "STAT", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = transaction_stat, .writer = write_transaction_stat},
        {.command = "LIST", .argument_1_type = OPTIONAL, .argument_2_type = EMPTY, .handler = transaction_list, .writer = write_transaction_list},
        {.command = "RETR", .argument_1_type = REQUIRED, .argument_2_type = EMPTY, .handler = transaction_retr, .writer = write_transaction_retr},
        {.command = "DELE", .argument_1_type = REQUIRED, .argument_2_type = EMPTY, .handler = transaction_dele, .writer = write_transaction_dele},
        {.command = "NOOP", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = transaction_noop, .writer = write_transaction_noop},
        {.command = "RSET", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = transaction_rset, .writer = write_transaction_rset},
        {.command = "CAPA", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = transaction_capa, .writer = write_transaction_capa},
        {.command = "QUIT", .argument_1_type = EMPTY, .argument_2_type = EMPTY, .handler = transaction_quit, .writer = write_transaction_quit},
};

struct pop3_command * pop3_commands[] = {
    authorization_commands,
    transaction_commands
};

size_t pop3_commands_length[] = {
        sizeof(authorization_commands) / sizeof(authorization_commands[0]),
        sizeof(transaction_commands) / sizeof(transaction_commands[0])
};

stm_states read_command(struct selector_key * key, stm_states current_state) {
    connection_data connection = (connection_data) key->data;
    char * ptr;

    // TODO check if this work with wrtiting.
    if (!buffer_can_read(&connection->in_buffer_object)) {
        size_t write_bytes;
        ptr = (char *) buffer_write_ptr(&connection->in_buffer_object, &write_bytes);
        ssize_t n = recv(key->fd, ptr, write_bytes, 0);
        if (n == 0) {
            return QUIT;
        }
        buffer_write_adv(&connection->in_buffer_object, n);
    }

    size_t read_bytes;
    ptr = (char *) buffer_read_ptr(&connection->in_buffer_object, &read_bytes);

    for (int i = 0; i < read_bytes; i++) {
        const struct parser_event * event = parser_feed(connection->parser, ptr[i], connection);
        buffer_read_adv(&connection->in_buffer_object, 1);

        if (event->type == VALID_COMMAND) {
            printf("Command: %s\nArgument 1: %s\nArgument 2: %s\n", connection->current_command.command, connection->current_command.argument_1, connection->current_command.argument_2);
            for (int j = 0; j < pop3_commands_length[current_state]; j++) {
                struct pop3_command maybe_command = pop3_commands[current_state][j];
                if (strcmp(maybe_command.command, connection->current_command.command) == 0) {
                    if ((maybe_command.argument_1_type == REQUIRED && connection->current_command.argument_1_length > 0) ||
                        (maybe_command.argument_1_type == EMPTY && connection->current_command.argument_1_length == 0) ||
                        (maybe_command.argument_1_type == OPTIONAL)) {
                        if ((maybe_command.argument_2_type == REQUIRED && connection->current_command.argument_2_length > 0) ||
                            (maybe_command.argument_2_type == EMPTY && connection->current_command.argument_2_length == 0) ||
                            (maybe_command.argument_2_type == OPTIONAL)) {
                            stm_states next_state = maybe_command.handler(connection);
                            selector_set_interest_key(key, OP_WRITE);
                            return next_state;
                        } else {
                            printf("ERROR EN ARGUMENT 2\n");
                            clear_parser_buffers(&connection->current_command);
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
            bool saw_carriage_return = false;
            while (i < read_bytes) {
                char c = (char) buffer_read(&connection->in_buffer_object);
                if (c == '\r') {
                    saw_carriage_return = true;
                } else if (c == '\n') {
                    if (saw_carriage_return) {
                        return ERROR;
                    }
                } else {
                    saw_carriage_return = false;
                }
                i++;
            }
            return ERROR;
        }
    }

    return current_state;
}

stm_states write_command(struct selector_key * key, stm_states current_state) {
    connection_data connection = (connection_data) key->data;
    char * ptr;

    if (buffer_can_write(&connection->out_buffer_object)) {
        size_t write_bytes;
        ptr = (char *) buffer_write_ptr(&connection->out_buffer_object, &write_bytes);
        for (int j = 0; j < pop3_commands_length[current_state]; j++) {
            struct pop3_command maybe_command = pop3_commands[current_state][j];
            if (strcmp(maybe_command.command, connection->current_command.command) == 0) {
                stm_states next_state = maybe_command.writer(connection, ptr, &write_bytes);
                buffer_write_adv(&connection->out_buffer_object, (ssize_t) write_bytes);
                clear_parser_buffers(&connection->current_command);
                return next_state;
            }
        }
    }

    return current_state;
}

void stm_authorization_arrival(stm_states state, struct selector_key * key) {

}

void stm_authorization_departure(stm_states state, struct selector_key * key) {

}

stm_states stm_authorization_read(struct selector_key * key) {
    printf("ESTOY EN EL AUTHORIZATION STATE\n");
    return read_command(key, AUTHORIZATION);
}

stm_states stm_authorization_write(struct selector_key * key) {
    printf("ESTOY EN EL AUTHORIZATION STATE WRITE\n");
    return write_command(key, AUTHORIZATION);
}

void stm_transaction_arrival(stm_states state, struct selector_key * key) {

}

void stm_transaction_departure(stm_states state, struct selector_key * key) {

}

stm_states stm_transaction_read(struct selector_key * key) {
    printf("ESTOY EN EL TRANSACTION STATE\n");
    return read_command(key, TRANSACTION);
}

stm_states stm_transaction_write(struct selector_key * key) {
    printf("ESTOY EN EL TRANSACTION STATE WRITE\n");
    return write_command(key, TRANSACTION);
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
    printf("ME LLEGO UN ERROR\n");
    struct command * current_command = &((connection_data) key->data)->current_command;
    clear_parser_buffers(current_command);
    current_command->finished = true;
    selector_set_interest_key(key, OP_WRITE);
}

void stm_error_departure(stm_states state, struct selector_key * key) {

}

stm_states stm_error_read(struct selector_key * key) {

}

stm_states stm_error_write(struct selector_key * key) {
    char * message = "-ERR Invalid Command\r\n";

    connection_data connection = (connection_data) key->data;

    size_t write_bytes;
    char * ptr = (char *) buffer_write_ptr(&connection->out_buffer_object, &write_bytes);
    if (write_bytes >= strlen(message)) {
        strncpy(ptr, message, strlen(message));
        buffer_write_adv(&connection->out_buffer_object, (ssize_t) write_bytes);
        return AUTHORIZATION;
    }

    return ERROR;
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
