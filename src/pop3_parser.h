#ifndef POP3_PARSER_H
#define POP3_PARSER_H

#include <stddef.h>

void parser_initial_state_any(struct parser_event * ret, uint8_t c);

void parser_command_state_any(struct parser_event * ret, uint8_t c);
void parser_command_state_space(struct parser_event * ret, uint8_t c);

#endif
