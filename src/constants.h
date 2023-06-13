#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stm.h>

#define MAX_CONCURRENT_CONNECTIONS 1024
#define MAX_QUEUED_CONNECTIONS 32
#define BUFFER_SIZE 512

#define COMMAND_LENGTH 4
#define MIN_COMMAND_LENGTH 3
#define ARGUMENT_LENGTH 254

typedef enum {
    AUTHORIZATION = 0,
    TRANSACTION,
    UPDATE,
    ERROR,
    QUIT,
    STM_STATES_COUNT
} stm_states;

typedef enum {
    COMMAND = 0,
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

#endif
