#include "client.h"
#include <parser.h>
#include "../server_constants.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include "client_parser.h"
#include "client_commands.h"

extern struct args args;

static const struct parser_state_transition client_parser_header_state[] = {
        {.when = '\n', .dest = CLIENT_COMMAND, .act1 = client_parser_header_state_line_feed},
        {.when = ANY, .dest = CLIENT_HEADER, .act1 = client_parser_header_state_any}
};

static const struct parser_state_transition client_parser_command_state[] = {
        {.when = ' ', .dest = CLIENT_ID, .act1 = client_parser_command_state_space},
        {.when = ANY, .dest = CLIENT_COMMAND, .act1 = client_parser_command_state_any}
};

static const struct parser_state_transition client_parser_id_state[] = {
        {.when = ' ', .dest = CLIENT_TOKEN, .act1 = client_parser_id_state_space},
        {.when = ANY, .dest = CLIENT_ID, .act1 = client_parser_id_state_any}
};

static const struct parser_state_transition client_parser_token_state[] = {
        {.when = '\n', .dest = CLIENT_CONTENT, .act1 = client_parser_token_state_line_feed},
        {.when = ANY, .dest = CLIENT_TOKEN, .act1 = client_parser_token_state_any}
};

static const struct parser_state_transition client_parser_content_state[] = {
        {.when = '\n', .dest = CLIENT_COMMAND, .act1 = client_parser_content_state_line_feed},
        {.when = ANY, .dest = CLIENT_CONTENT, .act1 = client_parser_content_state_any}
};

static const struct parser_state_transition * client_parser_state_table[] = {
        client_parser_header_state,
        client_parser_command_state,
        client_parser_id_state,
        client_parser_token_state,
        client_parser_content_state
};

static const size_t client_parser_state_n[] = {
        sizeof(client_parser_header_state) / sizeof(client_parser_header_state[0]),
        sizeof(client_parser_command_state) / sizeof(client_parser_command_state[0]),
        sizeof(client_parser_id_state) / sizeof(client_parser_id_state[0]),
        sizeof(client_parser_token_state) / sizeof(client_parser_token_state[0]),
        sizeof(client_parser_content_state) / sizeof(client_parser_content_state[0]),
};

static const struct parser_definition client_parser_definition = {
        .states = client_parser_state_table,
        .states_count = CLIENT_PARSER_STATES_COUNT,
        .start_state = CLIENT_HEADER,
        .states_n = client_parser_state_n
};

static const struct client_directive client_directives[] = {
        {.command = 'U', .content_type = REQUIRED, .handler = add_user},
        {.command = 'C', .content_type = REQUIRED, .handler = change_password},
        {.command = 'R', .content_type = REQUIRED, .handler = remove_user},
        {.command = 'D', .content_type = REQUIRED, .handler = change_directory},
        {.command = 'M', .content_type = REQUIRED, .handler = change_max_mails},
        {.command = 'S', .content_type = EMPTY, .handler = get_statistics},
        {.command = 'L', .content_type = EMPTY, .handler = list_users},
};

static const size_t client_directives_length = sizeof(client_directives) / sizeof(client_directives[0]);

void receive_client_directive(struct selector_key * key) {
    struct sockaddr_in client;
    unsigned int client_length = sizeof(client);

    char read_buffer[CLIENT_BUFFER_SIZE] = {0};
    char write_buffer[CLIENT_BUFFER_SIZE] = {0};

    ssize_t n = recvfrom(key->fd, read_buffer, CLIENT_BUFFER_SIZE, 0, (struct sockaddr *) &client, &client_length);
    if (n <= 0) {
        return;
    }

    printf("Me llego un mensaje\n");

    struct parser * parser = parser_init(parser_no_classes(), &client_parser_definition);
    struct client_command command = {0};
    for (ssize_t i = 0; i < n; i++) {
        const struct parser_event * event = parser_feed(parser, read_buffer[i], &command);
        if (event->type == VALID_COMMAND) {
            if (strcmp(args.token, command.token) != 0) {
                command.response_code = INVALID_AUTHENTICATION;
                write_error(write_buffer, &command);
                goto send;
            }
            for (size_t j = 0; j < client_directives_length; j++) {
                if (client_directives[j].command == command.command) {
                    if ((client_directives[j].content_type == EMPTY && command.content_length == 0) ||
                        (client_directives[j].content_type == REQUIRED && command.content_length > 0)) {
                        client_directives[j].handler(write_buffer, &command);
                    } else {
                        command.response_code = GENERIC_ERROR;
                        write_error(write_buffer, &command);
                    }
                    goto send;
                }
            }
            command.response_code = GENERIC_ERROR;
            write_error(write_buffer, &command);
            goto send;
        } else if (event->type == INVALID_COMMAND) {
            if (command.id_error) {
                goto finally;
            }
            write_error(write_buffer, &command);
        }
    }

    printf("%s\n", write_buffer);

    send:
    sendto(key->fd, write_buffer, strlen(write_buffer), 0, (struct sockaddr *) &client, client_length);
    finally:
    parser_destroy(parser);
}
