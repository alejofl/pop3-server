#ifndef CLIENT_PARSER_H
#define CLIENT_PARSER_H

#include <parser.h>
#include "../../constants.h"

void client_parser_header_state_line_feed(struct parser_event * ret, uint8_t c, void * data);
void client_parser_header_state_any(struct parser_event * ret, uint8_t c, void * data);

void client_parser_command_state_space(struct parser_event * ret, uint8_t c, void * data);
void client_parser_command_state_any(struct parser_event * ret, uint8_t c, void * data);

void client_parser_id_state_space(struct parser_event * ret, uint8_t c, void * data);
void client_parser_id_state_any(struct parser_event * ret, uint8_t c, void * data);

void client_parser_token_state_line_feed(struct parser_event * ret, uint8_t c, void * data);
void client_parser_token_state_any(struct parser_event * ret, uint8_t c, void * data);

void client_parser_content_state_line_feed(struct parser_event * ret, uint8_t c, void * data);
void client_parser_content_state_any(struct parser_event * ret, uint8_t c, void * data);

#endif
