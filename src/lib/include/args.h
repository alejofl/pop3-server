#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <stdbool.h>

#define MAX_USERS 732

struct users {
    char * name;
    char * pass;
};

struct args {
    unsigned short port;
    char * mail_directory;
    struct users users[MAX_USERS];
};

void parse_args(const int argc, char ** argv, struct args * args);

#endif

