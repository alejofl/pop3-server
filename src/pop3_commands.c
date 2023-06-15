#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include "constants.h"
#include "pop3_commands.h"

extern struct args args;

void read_from_mail_file(struct selector_key * file_key) {
    connection_data connection = (connection_data) file_key->data;

    size_t write_bytes;
    char * ptr = (char *) buffer_write_ptr(&connection->current_command.mail_buffer_object, &write_bytes);
    ssize_t n = read(connection->current_command.mail_fd, ptr, write_bytes);
    // Si no puede leer entonces explota
    buffer_write_adv(&connection->current_command.mail_buffer_object, n);
    if (n == 0) {
        selector_unregister_fd(file_key->s, connection->current_command.mail_fd);
        close(connection->current_command.mail_fd);
        connection->current_command.mail_fd = -1;
    } else  {
        selector_set_interest_key(file_key, OP_NOOP);
    }
    selector_set_interest(file_key->s, connection->current_command.connection_fd, OP_WRITE);
}

struct fd_handler mail_file_handler = {
        .handle_read = read_from_mail_file
};

stm_states authorization_user(struct selector_key * key, connection_data connection) {
    connection->current_command.finished = false;

    if (strlen(connection->current_command.argument_1) > USERNAME_SIZE) {
        connection->current_command.error = true;
        return AUTHORIZATION;
    }

    for (int i = 0; i < args.users_count; i++) {
        if (strcmp(args.users[i].name, connection->current_command.argument_1) == 0) {
            connection->current_session.maildir[0] = '\0';
            strcat(connection->current_session.maildir, args.mail_directory);
            strcat(connection->current_session.maildir, "/");
            strcat(connection->current_session.maildir, connection->current_command.argument_1);
            strcat(connection->current_session.maildir, "/cur");
            DIR * directory = opendir(connection->current_session.maildir);
            if (directory == NULL) {
                connection->current_command.error = true;
                connection->current_session.maildir[0] = '\0';
                return AUTHORIZATION;
            }
            closedir(directory);

            strcpy(connection->current_session.username, connection->current_command.argument_1);
            connection->current_command.error = false;
            return AUTHORIZATION;
        }
    }

    connection->current_command.error = true;
    return AUTHORIZATION;
}

stm_states authorization_pass(struct selector_key * key, connection_data connection) {
    connection->current_command.finished = false;

    bool error = false;
    for (int i = 0; !error && i < args.users_count; i++) {
        if (strcmp(args.users[i].name, connection->current_session.username) == 0) {
            if (args.users[i].logged_in) {
                connection->current_command.error = true;
                connection->current_session.username[0] = '\0';
                connection->current_session.maildir[0] = '\0';
                return AUTHORIZATION;
            } else {
                args.users[i].logged_in = true;
            }
            error = strcmp(args.users[i].pass, connection->current_command.argument_1) != 0;
            if (error) {
                connection->current_session.username[0] = '\0';
                connection->current_session.maildir[0] = '\0';
                args.users[i].logged_in = false;
            }
        }
    }
    connection->current_command.error = error;
    return AUTHORIZATION;
}

stm_states authorization_capa(struct selector_key * key, connection_data connection) {
    connection->current_command.finished = false;
    return AUTHORIZATION;
}

stm_states authorization_quit(struct selector_key * key, connection_data connection) {
    connection->current_command.finished = false;
    return AUTHORIZATION;
}

stm_states transaction_stat(struct selector_key * key, connection_data connection) {
    connection->current_command.finished = false;
    return TRANSACTION;
}

stm_states transaction_list(struct selector_key * key, connection_data connection) {
    connection->current_command.finished = false;
    connection->current_command.response_index = 0;
    connection->current_command.sent_title = false;
    return TRANSACTION;
}

stm_states transaction_retr(struct selector_key * key, connection_data connection) {
    connection->current_command.finished = false;
    connection->current_command.error = false;
    connection->current_command.mail_fd = -1;
    connection->current_command.crlf_flag = ANY_CHARACTER;
    connection->current_command.sent_title = false;

    if (connection->current_command.argument_1_length > 0) {
        char * end;
        long argument = strtol(connection->current_command.argument_1, &end, 10);
        if (end[0] != '\0' || argument - 1 >= connection->current_session.mail_count || connection->current_session.mails[argument - 1].deleted) {
            connection->current_command.error = true;
            return TRANSACTION;
        }

        connection->current_command.connection_fd = key->fd;
        int fd = open(connection->current_session.mails[argument - 1].path, O_RDONLY);
        connection->current_command.mail_fd = fd;
        if (fd == -1) {
            connection->current_command.error = true;
            return TRANSACTION;
        }
        if (selector_register(key->s, fd, &mail_file_handler, OP_NOOP, connection) != SELECTOR_SUCCESS) {
            close(fd);
            connection->current_command.error = true;
            return TRANSACTION;
        }
        selector_set_interest_key(key, OP_NOOP);
        return TRANSACTION;
    }
    connection->current_command.error = true;
    return TRANSACTION;
}

stm_states transaction_dele(struct selector_key * key, connection_data connection) {
    connection->current_command.finished = false;
    return TRANSACTION;
}

stm_states transaction_noop(struct selector_key * key, connection_data connection) {
    connection->current_command.finished = false;
    return TRANSACTION;
}

stm_states transaction_rset(struct selector_key * key,connection_data connection) {
    connection->current_command.finished = false;
    size_t maildir_size = 0;
    for (int i = 0; i < connection->current_session.mail_count; i++) {
        connection->current_session.mails[i].deleted = false;
        maildir_size += connection->current_session.mails[i].size;
    }
    connection->current_session.maildir_size = maildir_size;
    return TRANSACTION;
}

stm_states transaction_capa(struct selector_key * key,connection_data connection) {
    connection->current_command.finished = false;
    return TRANSACTION;
}

stm_states transaction_quit(struct selector_key * key,connection_data connection) {
    connection->current_command.finished = false;
    connection->current_command.error = false;
    for (int i = 0; i < connection->current_session.mail_count; i++) {
        if (connection->current_session.mails[i].deleted) {
            int result = remove(connection->current_session.mails[i].path);
            if (result == -1) {
                connection->current_command.error = true;
            }
        }
    }
    return TRANSACTION;
}

// -------- WRITE HANDLERS -------

stm_states write_authorization_user(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * message = "+OK Valid mailbox";
    size_t message_length = strlen(message);
    char * error_message = "-ERR Invalid mailbox";
    size_t error_message_length = strlen(error_message);

    if (connection->current_command.error) {
        if (error_message_length > *available_space - END_LINE_LENGTH) {
            return AUTHORIZATION;
        }
        strncpy(destination, error_message, error_message_length);
        strncpy(destination + error_message_length, END_LINE, END_LINE_LENGTH);
        *available_space = error_message_length + END_LINE_LENGTH;
    } else {
        if (message_length > *available_space - END_LINE_LENGTH) {
            return AUTHORIZATION;
        }
        strncpy(destination, message, message_length);
        strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
        *available_space = message_length + END_LINE_LENGTH;
    }
    connection->current_command.finished = true;
    return AUTHORIZATION;
}

stm_states write_authorization_pass(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * message = "+OK Logged in and mailbox ready";
    size_t message_length = strlen(message);
    char * error_message = "-ERR Invalid password or unable to lock mailbox";
    size_t error_message_length = strlen(error_message);

    if (connection->current_command.error) {
        if (error_message_length > *available_space - END_LINE_LENGTH) {
            return AUTHORIZATION;
        }
        strncpy(destination, error_message, error_message_length);
        strncpy(destination + error_message_length, END_LINE, END_LINE_LENGTH);
        *available_space = error_message_length + END_LINE_LENGTH;
        connection->current_command.finished = true;
        return AUTHORIZATION;
    }
    if (message_length > *available_space - END_LINE_LENGTH) {
        return AUTHORIZATION;
    }
    strncpy(destination, message, message_length);
    strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
    *available_space = message_length + END_LINE_LENGTH;
    connection->current_command.finished = true;
    return TRANSACTION;
}

stm_states write_authorization_capa(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * message = "+OK\r\nUSER\r\nPIPELINING\r\n.";
    size_t message_length = strlen(message);

    if (message_length > *available_space - END_LINE_LENGTH) {
        return AUTHORIZATION;
    }
    strncpy(destination, message, message_length);
    strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
    *available_space = message_length + END_LINE_LENGTH;
    connection->current_command.finished = true;
    return AUTHORIZATION;
}

stm_states write_authorization_quit(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * message = "+OK Have a nice day";
    size_t message_length = strlen(message);

    if (message_length > *available_space - END_LINE_LENGTH) {
        return AUTHORIZATION;
    }
    strncpy(destination, message, message_length);
    strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
    *available_space = message_length + END_LINE_LENGTH;
    connection->current_command.finished = true;
    return QUIT;
}

stm_states write_transaction_stat(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char message[BUFFER_SIZE];

    size_t real_mail_count = 0;
    for (int i = 0; i < connection->current_session.mail_count; i++) {
        if (!connection->current_session.mails[i].deleted) {
            real_mail_count++;
        }
    }

    size_t message_length = sprintf(message, "+OK %zu %zu", real_mail_count, connection->current_session.maildir_size);

    if (message_length > *available_space - END_LINE_LENGTH) {
        return TRANSACTION;
    }
    strncpy(destination, message, message_length);
    strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
    *available_space = message_length + END_LINE_LENGTH;
    connection->current_command.finished = true;
    return TRANSACTION;
}

stm_states write_transaction_list(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * error_message = "-ERR No such message";
    size_t error_message_length = strlen(error_message);

    if (connection->current_command.argument_1_length > 0) {
        char * end;
        long argument = strtol(connection->current_command.argument_1, &end, 10);
        if (end[0] != '\0' || argument - 1 >= connection->current_session.mail_count) {
            if (error_message_length > *available_space - END_LINE_LENGTH) {
                return TRANSACTION;
            }
            strncpy(destination, error_message, error_message_length);
            strncpy(destination + error_message_length, END_LINE, END_LINE_LENGTH);
            *available_space = error_message_length + END_LINE_LENGTH;
            connection->current_command.finished = true;
            return TRANSACTION;
        }

        char message[BUFFER_SIZE];
        size_t message_length = sprintf(message, "+OK %zu %zu", argument, connection->current_session.mails[argument - 1].size);

        if (message_length > *available_space - END_LINE_LENGTH) {
            return TRANSACTION;
        }
        strncpy(destination, message, message_length);
        strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
        *available_space = message_length + END_LINE_LENGTH;
        connection->current_command.finished = true;
        return TRANSACTION;
    }
    char message[BUFFER_SIZE] = {0};
    size_t message_length = 0;
    if (!connection->current_command.sent_title) {
        char * ok = "+OK\r\n";
        size_t ok_length = strlen(ok);
        if (ok_length > *available_space) {
            return TRANSACTION;
        }
        strncpy(message, ok, ok_length);
        *available_space -= ok_length;
        message_length += ok_length;
        connection->current_command.sent_title = true;
    }
    while (connection->current_command.response_index < connection->current_session.mail_count) {
        if (!connection->current_session.mails[connection->current_command.response_index].deleted) {
            char line[BUFFER_SIZE];
            size_t line_length = sprintf(line, "%zu %zu\r\n", connection->current_command.response_index + 1, connection->current_session.mails[connection->current_command.response_index].size);
            if (line_length > *available_space) {
                break;
            }
            strncpy(message + message_length, line, line_length);
            *available_space -= line_length;
            message_length += line_length;
        }
        connection->current_command.response_index++;
    }
    if (connection->current_command.response_index == connection->current_session.mail_count) {
        char * end = ".\r\n";
        size_t end_length = strlen(end);
        if (end_length <= *available_space) {
            strncpy(message + message_length, end, end_length);
            *available_space -= end_length;
            message_length += end_length;
            connection->current_command.finished = true;
        }
    }

    strncpy(destination, message, message_length);
    *available_space = message_length;
    return TRANSACTION;
}

stm_states write_transaction_retr(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * error_message = "-ERR No such message";
    size_t error_message_length = strlen(error_message);

    if (connection->current_command.error) {
        if (error_message_length > *available_space - END_LINE_LENGTH) {
            return TRANSACTION;
        }
        strncpy(destination, error_message, error_message_length);
        strncpy(destination + error_message_length, END_LINE, END_LINE_LENGTH);
        *available_space = error_message_length + END_LINE_LENGTH;
        connection->current_command.finished = true;
        return TRANSACTION;
    }

    if (!connection->current_command.sent_title) {
        char * ok = "+OK message follows\r\n";
        size_t ok_length = strlen(ok);
        if (ok_length > *available_space) {
            return TRANSACTION;
        }
        strncpy(destination, ok, ok_length);
        *available_space = ok_length;
        connection->current_command.sent_title = true;
        selector_set_interest_key(key, OP_NOOP);
        selector_set_interest(key->s, connection->current_command.mail_fd, OP_READ);
        return TRANSACTION;
    }

    while (buffer_can_write(&connection->out_buffer_object)) {
        if (buffer_can_read(&connection->current_command.mail_buffer_object)) {
            char c;
            if (connection->current_command.crlf_flag == DOT) {
                c = '.';
            } else {
                c = (char) buffer_read(&connection->current_command.mail_buffer_object);
            }
            if (c == '\r') {
                connection->current_command.crlf_flag = CR;
            } else if (c == '\n' && connection->current_command.crlf_flag == CR) {
                connection->current_command.crlf_flag = LF;
            } else if (c == '.' && connection->current_command.crlf_flag == LF) {
                buffer_write(&connection->out_buffer_object, '.');
                if (!buffer_can_write(&connection->out_buffer_object)) {
                    connection->current_command.crlf_flag = DOT;
                    return TRANSACTION;
                }
            } else {
                connection->current_command.crlf_flag = ANY_CHARACTER;
            }
            buffer_write(&connection->out_buffer_object, c);
        } else {
            if (connection->current_command.mail_fd == -1) {
                char retr_ending[5];
                size_t retr_ending_length = 0;
                if (connection->current_command.crlf_flag != LF) {
                    strcpy(retr_ending, "\r\n");
                    retr_ending_length = 2;
                }
                strcpy(retr_ending + retr_ending_length, ".\r\n");
                retr_ending_length += 3;

                size_t write_bytes;
                char * ptr = (char *) buffer_write_ptr(&connection->out_buffer_object, &write_bytes);
                if (retr_ending_length > write_bytes) {
                    return TRANSACTION;
                }
                strncpy(ptr, retr_ending, retr_ending_length);
                buffer_write_adv(&connection->out_buffer_object, (ssize_t) retr_ending_length);

                connection->current_command.finished = true;
                connection->current_command.crlf_flag = ANY_CHARACTER;
                *available_space = 0;
                return TRANSACTION;
            } else {
                break;
            }
        }
    }

    if (!buffer_can_read(&connection->current_command.mail_buffer_object)) {
        selector_set_interest_key(key, OP_NOOP);
        selector_set_interest(key->s, connection->current_command.mail_fd, OP_READ);
    }
    *available_space = 0;
    return TRANSACTION;
}

stm_states write_transaction_dele(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * message = "+OK Message deleted";
    size_t message_length = strlen(message);
    char * error_message = "-ERR No such message";
    size_t error_message_length = strlen(error_message);

    char * end;
    long argument = strtol(connection->current_command.argument_1, &end, 10);
    if (end[0] != '\0' || argument - 1 >= connection->current_session.mail_count || connection->current_session.mails[argument - 1].deleted == true) {
        if (error_message_length > *available_space - END_LINE_LENGTH) {
            return TRANSACTION;
        }
        strncpy(destination, error_message, error_message_length);
        strncpy(destination + error_message_length, END_LINE, END_LINE_LENGTH);
        *available_space = error_message_length + END_LINE_LENGTH;
        connection->current_command.finished = true;
        return TRANSACTION;
    }

    connection->current_session.mails[argument - 1].deleted = true;

    if (message_length > *available_space - END_LINE_LENGTH) {
        return TRANSACTION;
    }
    strncpy(destination, message, message_length);
    strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
    *available_space = message_length + END_LINE_LENGTH;
    connection->current_session.maildir_size -= connection->current_session.mails[argument - 1].size;
    connection->current_command.finished = true;
    return TRANSACTION;
}

stm_states write_transaction_noop(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * message = "+OK";
    size_t message_length = strlen(message);

    if (message_length > *available_space - END_LINE_LENGTH) {
        return TRANSACTION;
    }
    strncpy(destination, message, message_length);
    strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
    *available_space = message_length + END_LINE_LENGTH;
    connection->current_command.finished = true;
    return TRANSACTION;
}

stm_states write_transaction_rset(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * message = "+OK";
    size_t message_length = strlen(message);

    if (message_length > *available_space - END_LINE_LENGTH) {
        return TRANSACTION;
    }
    strncpy(destination, message, message_length);
    strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
    *available_space = message_length + END_LINE_LENGTH;
    connection->current_command.finished = true;
    return TRANSACTION;
}

stm_states write_transaction_capa(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * message = "+OK\r\nUSER\r\nPIPELINING\r\n.";
    size_t message_length = strlen(message);

    if (message_length > *available_space - END_LINE_LENGTH) {
        return TRANSACTION;
    }
    strncpy(destination, message, message_length);
    strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
    *available_space = message_length + END_LINE_LENGTH;
    connection->current_command.finished = true;
    return TRANSACTION;
}

stm_states write_transaction_quit(struct selector_key * key, connection_data connection, char * destination, size_t * available_space) {
    char * message = "+OK Have a nice day";
    size_t message_length = strlen(message);
    char * error_message = "-ERR Some deleted messages were not removed";
    size_t error_message_length = strlen(error_message);

    if (connection->current_command.error) {
        if (error_message_length > *available_space - END_LINE_LENGTH) {
            return TRANSACTION;
        }
        strncpy(destination, error_message, error_message_length);
        strncpy(destination + error_message_length, END_LINE, END_LINE_LENGTH);
        *available_space = error_message_length + END_LINE_LENGTH;
    } else {
        if (message_length > *available_space - END_LINE_LENGTH) {
            return TRANSACTION;
        }
        strncpy(destination, message, message_length);
        strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
        *available_space = message_length + END_LINE_LENGTH;
    }
    connection->current_command.finished = true;
    return QUIT;
}
