#include <parser.h>
#include <stdio.h>
#include "constants.h"
#include "pop3_parser.h"

void parser_initial_state_any(struct parser_event * ret, const uint8_t c) {
    ret->type = UNDEFINED;
}

void parser_command_state_any(struct parser_event * ret, const uint8_t c) {
    ret->type = UNDEFINED;
    printf("Me llegó una %c\n", c);
}

void parser_command_state_space(struct parser_event * ret, const uint8_t c) {
    ret->type = VALID_COMMAND;
    printf("Terminó el comando\n");
}