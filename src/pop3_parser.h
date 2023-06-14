#ifndef POP3_PARSER_H
#define POP3_PARSER_H

#include <stddef.h>

void clear_parser_buffers(struct command * command);

void parser_command_state_any(struct parser_event * ret, uint8_t c, connection_data connection);
void parser_command_state_space(struct parser_event * ret, uint8_t c, connection_data connection);
void parser_command_state_carriage_return(struct parser_event * ret, uint8_t c, connection_data connection);

void parser_argument_1_state_space(struct parser_event * ret, uint8_t c,connection_data connection);
void parser_argument_1_state_any(struct parser_event * ret, uint8_t c, connection_data connection);
void parser_argument_1_state_carriage_return(struct parser_event * ret, uint8_t c, connection_data connection);

void parser_argument_2_state_carriage_return(struct parser_event * ret, uint8_t c, connection_data connection);
void parser_argument_2_state_any(struct parser_event * ret, uint8_t c, connection_data connection);

void parser_end_state_line_feed(struct parser_event * ret, uint8_t c, connection_data connection);
void parser_end_state_any(struct parser_event * ret, uint8_t c, connection_data connection);

#endif
