#include <string.h>
#include <ctype.h>
#include "client.h"
#include "client_parser.h"

void client_parser_header_state_line_feed(struct parser_event * ret, uint8_t c, void * data) {
    struct client_command * command = (struct client_command *) data;
    command->header[command->header_length] = '\0';
    if (strcmp(command->header, "turtle v1") != 0) {
        ret->type = INVALID_COMMAND;
        command->response_code = INVALID_VERSION;
        return;
    }
    ret->type = UNDEFINED;
}

void client_parser_header_state_any(struct parser_event * ret, uint8_t c, void * data) {
    struct client_command * command = (struct client_command *) data;
    if (command->header_length + 1 > CLIENT_HEADER_LENGTH) {
        ret->type = INVALID_COMMAND;
        command->response_code = GENERIC_ERROR;
        return;
    }
    command->header[command->header_length] = (char) c;
    command->header_length++;
    ret->type = UNDEFINED;
}

void client_parser_command_state_space(struct parser_event * ret, uint8_t c, void * data) {
    struct client_command * command = (struct client_command *) data;
    if (command->command != 'U' && command->command != 'C' &&
        command->command != 'R' && command->command != 'D' &&
        command->command != 'M' && command->command != 'S' &&
        command->command != 'L') {
        ret->type = INVALID_COMMAND;
        command->response_code = GENERIC_ERROR;
        return;
    }
    ret->type = UNDEFINED;
}

void client_parser_command_state_any(struct parser_event * ret, uint8_t c, void * data) {
    struct client_command * command = (struct client_command *) data;
    if (command->command != 0) {
        ret->type = INVALID_COMMAND;
        command->response_code = GENERIC_ERROR;
        return;
    }
    command->command = (char) c;
    ret->type = UNDEFINED;
}

void client_parser_id_state_space(struct parser_event * ret, uint8_t c, void * data) {
    struct client_command * command = (struct client_command *) data;
    command->id[command->id_length] = '\0';
    long id = strtol(command->id, NULL, 10);
    if (id > INT32_MAX) {
        ret->type = INVALID_COMMAND;
        command->response_code = GENERIC_ERROR;
        command->id_error = true;
        return;
    }
    ret->type = UNDEFINED;
}

void client_parser_id_state_any(struct parser_event * ret, uint8_t c, void * data) {
    struct client_command * command = (struct client_command *) data;
    if (command->id_length + 1 > CLIENT_ID_LENGTH || c < '0' || c > '9') {
        ret->type = INVALID_COMMAND;
        command->response_code = GENERIC_ERROR;
        command->id_error = true;
        return;
    }
    command->id[command->id_length] = (char) c;
    command->id_length++;
    ret->type = UNDEFINED;
}

void client_parser_token_state_line_feed(struct parser_event * ret, uint8_t c, void * data) {
    struct client_command * command = (struct client_command *) data;
    command->token[command->token_length] = '\0';
    ret->type = UNDEFINED;
}

void client_parser_token_state_any(struct parser_event * ret, uint8_t c, void * data) {
    struct client_command * command = (struct client_command *) data;
    if (command->token_length + 1 > CLIENT_TOKEN_LENGTH || !isalnum(c)) {
        ret->type = INVALID_COMMAND;
        command->response_code = GENERIC_ERROR;
        return;
    }
    command->token[command->token_length] = (char) c;
    command->token_length++;
    ret->type = UNDEFINED;
}

void client_parser_content_state_line_feed(struct parser_event * ret, uint8_t c, void * data) {
    struct client_command * command = (struct client_command *) data;
    command->content[command->content_length] = '\0';
    ret->type = VALID_COMMAND;
}

void client_parser_content_state_any(struct parser_event * ret, uint8_t c, void * data) {
    struct client_command * command = (struct client_command *) data;
    if (command->content_length + 1 > CLIENT_CONTENT_LENGTH) {
        ret->type = INVALID_COMMAND;
        command->response_code = GENERIC_ERROR;
        return;
    }
    command->content[command->content_length] = (char) c;
    command->content_length++;
    ret->type = UNDEFINED;
}
