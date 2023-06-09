#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SERVER_PORT 61655
#define CLIENT_PORT 62622

#define CLIENT_CONTENT_LENGTH 255
#define CLIENT_BUFFER_SIZE 8192
#define CLIENT_TOKEN_LENGTH 6

typedef enum {
    ALL_GOOD = 10,
    NOT_EXISTS = 50,
    ALREADY_EXISTS = 51,
    MAXIMUM_REACHED = 52,
    INVALID_VERSION = 97,
    INVALID_AUTHENTICATION = 98,
    GENERIC_ERROR = 99
} client_responses;

#endif
