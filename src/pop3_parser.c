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

// TODO es mi estado inicial, no me interesa a donde voy porque avanzo al siguiente estado que es command.
// TODO tenemos que definir una estructura a utilizar para guardar los comandos
void parser_command_state_space(struct parser_event * ret, const uint8_t c) {
    ret->type = VALID_COMMAND;
    printf("Terminó el comando\n");
}

void parser_argument_1_state_space(struct parser_event * ret, uint8_t c) {
    return;
}

void parser_argument_1_state_any(struct parser_event * ret, uint8_t c) {
    return;
}

void parser_argument_2_state_space(struct parser_event * ret, uint8_t c) {
    return;
}

void parser_argument_2_state_any(struct parser_event * ret, uint8_t c) {
    return;
}

// TODO lo unico que hace este es seguir con la rocola
void parser_end_state_space(struct parser_event * ret, uint8_t c) {
    return;
}

void parser_end_state_any(struct parser_event * ret, uint8_t c) {
    ret->type = ;
    return;
}