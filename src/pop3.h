#ifndef POP3_H
#define POP3_H

#include "constants.h"
#include <selector.h>
#include <stm.h>
#include <buffer.h>

struct connection_data {
    struct buffer buffer_object;
    char buffer[BUFFER_SIZE];
    struct parser * parser;
    struct state_machine stm;
};

typedef struct connection_data * connection_data;

void accept_pop_connection(struct selector_key * key);

#endif
