#ifndef CONSTANTS_H
#define CONSTANTS_H

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
