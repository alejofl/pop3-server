#ifndef CLIENT_H
#define CLIENT_H

#include <selector.h>
#include "constants.h"
#include "server_constants.h"

typedef enum {
    EMPTY,
    REQUIRED
} content_type;

struct client_command {
    char header[CLIENT_HEADER_LENGTH + 1];
    char command;
    char id[CLIENT_ID_LENGTH + 1];
    char token[CLIENT_TOKEN_LENGTH + 1];
    char content[CLIENT_CONTENT_LENGTH];
    size_t header_length;
    size_t id_length;
    size_t token_length;
    size_t content_length;

    client_responses response_code;
    bool id_error;
};

struct client_directive {
    char command;
    content_type content_type;
    void (* handler)(char * buffer, struct client_command * command);
};

void receive_client_directive(struct selector_key * key);

#endif
