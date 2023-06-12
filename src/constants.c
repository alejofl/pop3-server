#include <stddef.h>
#include "constants.h"
#include "pop3_parser.h"

static const struct parser_state_transition parser_initial_state[] = {
    {.when = ANY, .dest = COMMAND, .act1 = parser_initial_state_any}
};

static const struct parser_state_transition parser_command_state[] = {
    {.when = ' ', .dest = ARGUMENT_1, .act1 = parser_command_state_any},
    {.when = ANY, .dest = COMMAND, .act1 = parser_command_state_space}
};

static const struct parser_state_transition parser_argument_1_state[] = {
    {.when = ' ', .dest = ARGUMENT_2, .act1 = NULL},
    {.when = ANY, .dest = ARGUMENT_1, .act1 = NULL}
};

static const struct parser_state_transition parser_argument_2_state[] = {
    {.when = '\r', .dest = END, .act1 = NULL},
    {.when = ANY, .dest = ARGUMENT_2, .act1 = NULL}
};

static const struct parser_state_transition parser_end_state[] = {
    {.when = '\n', .dest = INITIAL, .act1 = NULL},
    {.when = ANY, .dest = INITIAL, .act1 = NULL}
};

static const struct parser_state_transition * parser_state_table[] = {
    parser_initial_state,
    parser_command_state,
    parser_argument_1_state,
    parser_argument_2_state,
    parser_end_state
};

static const size_t parser_state_n[] = {
        sizeof(parser_initial_state) / sizeof(parser_initial_state[0]),
        sizeof(parser_command_state) / sizeof(parser_command_state[0]),
        sizeof(parser_argument_1_state) / sizeof(parser_argument_1_state[0]),
        sizeof(parser_argument_2_state) / sizeof(parser_argument_2_state[0]),
        sizeof(parser_end_state) / sizeof(parser_end_state[0]),
};

static const struct parser_definition parser_definition = {
    .states = parser_state_table,
    .states_count = PARSER_STATES_COUNT,
    .start_state = INITIAL,
    .states_n = parser_state_n
};
