#ifndef CLIENT_ARGS_H
#define CLIENT_ARGS_H

#include <stdbool.h>
#include <stdlib.h>
#include "../constants.h"

#define MAX_COMMANDS 64


struct command {
    char command;
    char content[CLIENT_CONTENT_LENGTH];
};

struct args {
    unsigned short port;
    char token[CLIENT_TOKEN_LENGTH + 1];
    struct command commands[MAX_COMMANDS];
    size_t commands_length;
};

void parse_args(const int argc, char ** argv, struct args * args);

#endif

