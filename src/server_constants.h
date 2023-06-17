#ifndef SERVER_CONSTANTS_H
#define SERVER_CONSTANTS_H

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

#define CLIENT_BUFFER_SIZE 8192
#define CLIENT_HEADER_LENGTH 9
#define CLIENT_TOKEN_LENGTH 6
#define CLIENT_ID_LENGTH 10
#define CLIENT_CONTENT_LENGTH 255

typedef enum {
    AUTHORIZATION = 0,
    TRANSACTION,
    ERROR,
    QUIT,
    STM_STATES_COUNT
} stm_states;

typedef enum {
    COMMAND = 0,
    ARGUMENT,
    END,
    PARSER_STATES_COUNT
} parser_states;

typedef enum {
    UNDEFINED,
    VALID_COMMAND,
    INVALID_COMMAND
} parser_event_types;

typedef enum {
    CLIENT_HEADER = 0,
    CLIENT_COMMAND,
    CLIENT_ID,
    CLIENT_TOKEN,
    CLIENT_CONTENT,
    CLIENT_PARSER_STATES_COUNT
} client_parser_states;

struct users {
    char name[USERNAME_SIZE];
    char pass[USERNAME_SIZE];
    bool logged_in;
};

struct args {
    unsigned short server_port;
    unsigned short client_port;
    char mail_directory[PATH_SIZE];
    size_t max_mails;
    struct users users[MAX_USERS];
    size_t users_count;
};

struct stats {
    unsigned long historical_connections;
    unsigned long concurrent_connections;
    unsigned long transferred_bytes;
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
    bool requested_quit;
};

typedef enum {
    ANY_CHARACTER,
    CR,
    LF,
    DOT
} crlf_flag;

#endif
