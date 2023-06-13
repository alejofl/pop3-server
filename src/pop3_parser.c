#include <parser.h>
#include <stdio.h>
#include "constants.h"
#include "pop3_parser.h"

void parser_initial_state_any(struct parser_event * ret, const uint8_t c, connection_data connection) {
    ret->type = UNDEFINED;
}

void parser_command_state_any(struct parser_event * ret, const uint8_t c, connection_data connection) {
    printf("Me llegó una %c\n", c);
    if (connection->current_command.command_length > COMMAND_LENGTH) {
        ret->type = INVALID_COMMAND;
        return;
    }
    ret->type = UNDEFINED;
    connection->current_command.command[connection->current_command.command_length] = (char) c;
    connection->current_command.command_length++;
}

void parser_command_state_space(struct parser_event * ret, const uint8_t c, connection_data connection) {
    if (connection->current_command.command_length < MIN_COMMAND_LENGTH) {
        ret->type = INVALID_COMMAND;
        return;
    }
    connection->current_command.command[connection->current_command.command_length] = '\0';
    ret->type = UNDEFINED;
    printf("Terminó el comando\n");
}

void parser_argument_1_state_space(struct parser_event * ret, uint8_t c, connection_data connection) {
    if (connection->current_command.command_length < 0) {
        ret->type = INVALID_COMMAND;
        return;
    }
    connection->current_command.command[connection->current_command.argument_1_length] = '\0';
    ret->type = UNDEFINED;
    printf("Terminó el argumento 1\n");
}

void parser_argument_1_state_any(struct parser_event * ret, uint8_t c, connection_data connection) {
    if (connection->current_command.argument_1_length > ARGUMENT_LENGTH) {
        ret->type = INVALID_COMMAND;
        return;
    }
    ret->type = UNDEFINED;
    connection->current_command.argument_1[connection->current_command.argument_1_length] = (char) c;
    connection->current_command.argument_1_length++;
}

void parser_argument_2_state_carriage_return(struct parser_event * ret, uint8_t c, connection_data connection) {
    if (connection->current_command.command_length < 0) {
        ret->type = INVALID_COMMAND;
        return;
    }
    connection->current_command.command[connection->current_command.argument_2_length] = '\0';
    ret->type = UNDEFINED;
    printf("Terminó el argumento 2\n");
}

void parser_argument_2_state_any(struct parser_event * ret, uint8_t c, connection_data connection) {
    if (connection->current_command.argument_2_length > ARGUMENT_LENGTH) {
        ret->type = INVALID_COMMAND;
        return;
    }
    ret->type = UNDEFINED;
    connection->current_command.argument_2[connection->current_command.argument_2_length] = (char) c;
    connection->current_command.argument_2_length++;
}

// TODO lo unico que hace este es seguir con la rocola
void parser_end_state_line_feed(struct parser_event * ret, uint8_t c, connection_data connection) {
    return;
}

void parser_end_state_any(struct parser_event * ret, uint8_t c, connection_data connection) {
    ret->type = ;
    return;
}