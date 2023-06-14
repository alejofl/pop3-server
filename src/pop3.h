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
    bool finished;
    bool error;
};

struct connection_data {
    struct buffer in_buffer_object;
    char in_buffer[BUFFER_SIZE];
    struct buffer out_buffer_object;
    char out_buffer[BUFFER_SIZE];
    struct parser * parser;
    struct state_machine stm;

    struct session current_session;
    struct command current_command;
    stm_states last_state;
};

typedef struct connection_data * connection_data;

void accept_pop_connection(struct selector_key * key);

#endif
