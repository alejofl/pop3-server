#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stm.h>

#define MAX_CONCURRENT_CONNECTIONS 1024
#define MAX_QUEUED_CONNECTIONS 32
#define BUFFER_SIZE 512

#define COMMAND_LENGTH 4
#define MIN_COMMAND_LENGTH 3
#define ARGUMENT_LENGTH 250

#define END_LINE "\r\n"
#define END_LINE_LENGTH 2

#define MAX_USERS 10
#define USERNAME_SIZE 256
#define PATH_SIZE 4096
#define INITIAL_MAILS_QTY 64

typedef enum {
    AUTHORIZATION = 0,
    TRANSACTION,
    ERROR,
    QUIT,
    STM_STATES_COUNT
} stm_states;

typedef enum {
    COMMAND = 0,
    ARGUMENT_1,
    END,
    PARSER_STATES_COUNT
} parser_states;

typedef enum {
    UNDEFINED,
    VALID_COMMAND,
    INVALID_COMMAND
} parser_event_types;

struct users {
    char * name;
    char * pass;
    bool logged_in;
};

struct args {
    unsigned short port;
    char * mail_directory;
    size_t max_mails;
    struct users users[MAX_USERS];
    size_t users_count;
};

struct mail {
    char path[PATH_SIZE];
    bool deleted;
    size_t size;
};

struct session {
    char username[USERNAME_SIZE];
    char maildir[PATH_SIZE];
    struct mail * mails;
    size_t mail_count;
    size_t maildir_size;
};

typedef enum {
    ANY_CHARACTER,
    CR,
    LF,
    DOT
} crlf_flag;

#endif
