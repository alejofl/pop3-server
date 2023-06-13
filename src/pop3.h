#ifndef POP3_H
#define POP3_H

#include "constants.h"
#include <selector.h>
#include <stm.h>
#include <buffer.h>

struct command {
    char command[COMMAND_LENGTH + 1];
    char argument_1[ARGUMENT_LENGTH + 1];
    char argument_2[ARGUMENT_LENGTH + 1];
    size_t command_length;
    size_t argument_1_length;
    size_t argument_2_length;
};

struct connection_data {
    struct buffer buffer_object;
    char buffer[BUFFER_SIZE];
    struct parser * parser;
    struct state_machine stm;

    struct command current_command;
};

typedef struct connection_data * connection_data;

void accept_pop_connection(struct selector_key * key);

#endif
