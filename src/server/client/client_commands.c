#include "client_commands.h"
#include "../server_constants.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern struct args args;
extern struct stats stats;

void write_error(char * buffer, struct client_command * command) {
    sprintf(buffer, "%d %s\n\n", command->response_code, command->id);
}

void write_success(char * buffer, char * content, struct client_command * command) {
    sprintf(buffer, "%d %s\n%s\n", command->response_code, command->id, content);
}

void add_user(char * buffer, struct client_command * command) {
    if (args.users_count == MAX_USERS) {
        command->response_code = MAXIMUM_REACHED;
        write_error(buffer, command);
        return;
    }

    char * p = strchr(command->content, ':');
    if (p == NULL) {
        command->response_code = GENERIC_ERROR;
        write_error(buffer, command);
        return;
    }
    *p = '\0';
    p++;
    char * name = command->content;
    char * pass = p;

    for (size_t i = 0; i < args.users_count; i++) {
        if (strcmp(name, args.users[i].name) == 0) {
            command->response_code = ALREADY_EXISTS;
            write_error(buffer, command);
            return;
        }
    }
    strcpy(args.users[args.users_count].name, name);
    strcpy(args.users[args.users_count].pass, pass);
    args.users_count++;
    command->response_code = ALL_GOOD;
    write_success(buffer, "", command);
}

void change_password(char * buffer, struct client_command * command) {
    char * p = strchr(command->content, ':');
    if (p == NULL) {
        command->response_code = GENERIC_ERROR;
        write_error(buffer, command);
        return;
    }
    *p = '\0';
    p++;
    char * name = command->content;
    char * pass = p;

    for (size_t i = 0; i < args.users_count; i++) {
        if (strcmp(name, args.users[i].name) == 0) {
            strcpy(args.users[args.users_count].pass, pass);
            command->response_code = ALL_GOOD;
            write_success(buffer, "", command);
            return;
        }
    }
    command->response_code = NOT_EXISTS;
    write_error(buffer, command);
}

void remove_user(char * buffer, struct client_command * command) {
    for (size_t i = 0; i < args.users_count; i++) {
        if (strcmp(command->content, args.users[i].name) == 0) {
            if (args.users[i].logged_in) {
                command->response_code = GENERIC_ERROR;
                write_error(buffer, command);
                return;
            }

            for (size_t j = i + 1; j < args.users_count; i++, j++) {
                strcpy(args.users[i].name, args.users[j].name);
                strcpy(args.users[i].pass, args.users[j].pass);
            }
            args.users_count--;
            command->response_code = ALL_GOOD;
            write_success(buffer, "", command);
            return;
        }
    }
    command->response_code = NOT_EXISTS;
    write_error(buffer, command);
}

void change_directory(char * buffer, struct client_command * command) {
    strcpy(args.mail_directory, command->content);
    command->response_code = ALL_GOOD;
    write_success(buffer, "", command);
}

void change_max_mails(char * buffer, struct client_command * command) {
    char * end;
    int max_mails = (int) strtol(command->content, &end, 10);
    if (max_mails < 1) {
        command->response_code = GENERIC_ERROR;
        write_error(buffer, command);
        return;
    }
    args.max_mails = max_mails;
    command->response_code = ALL_GOOD;
    write_success(buffer, "", command);
}

void get_statistics(char * buffer, struct client_command * command) {
    char content[CLIENT_CONTENT_LENGTH] = {0};
    sprintf(content, "HC:%zu CC:%zu TB:%zu", stats.historical_connections, stats.concurrent_connections, stats.transferred_bytes);
    command->response_code = ALL_GOOD;
    write_success(buffer, content, command);
}

void list_users(char * buffer, struct client_command * command) {
    char content[CLIENT_CONTENT_LENGTH] = {0};
    size_t content_length = 0;
    for (size_t i = 0; i < args.users_count; i++) {
        strcpy(content + content_length, args.users[i].name);
        content_length += strlen(args.users[i].name);
        strcpy(content + content_length, " ");
        content_length++;
    }
    command->response_code = ALL_GOOD;
    write_success(buffer, content, command);
}
