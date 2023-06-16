#include <parser.h>
#include <stdio.h>
#include "constants.h"
#include "pop3_parser.h"

void clear_parser_buffers(struct command * command) {
    command->command[0] = '\0';
    command->command_length = 0;
    command->argument[0] = '\0';
    command->argument_length = 0;
}

void parser_command_state_any(struct parser_event * ret, const uint8_t c, connection_data connection) {
    if (connection->current_command.command_length + 1 > COMMAND_LENGTH) {
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
}

void parser_command_state_carriage_return(struct parser_event * ret, uint8_t c, connection_data connection) {
    if (connection->current_command.command_length < MIN_COMMAND_LENGTH) {
        ret->type = INVALID_COMMAND;
        return;
    }
    connection->current_command.command[connection->current_command.command_length] = '\0';
    ret->type = UNDEFINED;
}

void parser_argument_state_any(struct parser_event * ret, uint8_t c, connection_data connection) {
    if (connection->current_command.argument_length > ARGUMENT_LENGTH) {
        ret->type = INVALID_COMMAND;
        return;
    }
    ret->type = UNDEFINED;
    connection->current_command.argument[connection->current_command.argument_length] = (char) c;
    connection->current_command.argument_length++;
}

void parser_argument_state_carriage_return(struct parser_event * ret, uint8_t c, connection_data connection) {
    if (connection->current_command.argument_length == 0) {
        ret->type = INVALID_COMMAND;
        return;
    }
    connection->current_command.argument[connection->current_command.argument_length] = '\0';
    ret->type = UNDEFINED;
}

void parser_end_state_line_feed(struct parser_event * ret, uint8_t c, connection_data connection) {
    ret->type = VALID_COMMAND;
}

void parser_end_state_any(struct parser_event * ret, uint8_t c, connection_data connection) {
    ret->type = INVALID_COMMAND;
}