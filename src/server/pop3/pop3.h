#ifndef POP3_H
#define POP3_H

#include "../server_constants.h"
#include "selector.h"
#include "stm.h"
#include "buffer.h"

struct command {
    char command[COMMAND_LENGTH + 1];
    char argument[ARGUMENT_LENGTH + 1];
    size_t command_length;
    size_t argument_length;

    bool finished;
    bool error;
    size_t response_index;
    int mail_fd;
    int connection_fd;
    crlf_flag crlf_flag;
    struct buffer mail_buffer_object;
    char mail_buffer[BUFFER_SIZE];
    bool sent_title;
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
