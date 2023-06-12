#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stm.h>
#include <parser.h>

#define MAX_CONCURRENT_CONNECTIONS 1024
#define MAX_QUEUED_CONNECTIONS 32
#define BUFFER_SIZE 512

typedef enum {
    AUTHORIZATION = 0,
    TRANSACTION = 1,
    UPDATE = 2,
    ERROR = 3,
    QUIT = 4,
    STM_STATES_COUNT = 5
} stm_states;

typedef enum {
    INITIAL = 0,
    COMMAND,
    ARGUMENT_1,
    ARGUMENT_2,
    END,
    PARSER_STATES_COUNT
} parser_states;

typedef enum {
    UNDEFINED,
    VALID_COMMAND,
    INVALID_COMMAND
} parser_event_types;

static const struct parser_definition parser_definition;

#endif
