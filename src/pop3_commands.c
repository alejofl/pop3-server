#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include "constants.h"
#include "pop3_commands.h"

extern struct args args;

stm_states authorization_user(connection_data connection) {
    connection->current_command.finished = false;

    if (strlen(connection->current_command.argument_1) > USERNAME_SIZE) {
        connection->current_command.error = true;
        return AUTHORIZATION;
    }

    for (int i = 0; i < args.users_count; i++) {
        if (strcmp(args.users[i].name, connection->current_command.argument_1) == 0) {
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

stm_states authorization_pass(connection_data connection) {
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

stm_states authorization_capa(connection_data connection) {
    connection->current_command.finished = false;
    return AUTHORIZATION;
}

stm_states authorization_quit(connection_data connection) {
    printf("AUHTORIZATION QUIT\n");
    return QUIT;
}

stm_states transaction_stat(connection_data connection) {
    connection->current_command.finished = false;
    return TRANSACTION;
}

stm_states transaction_list(connection_data connection) {
    connection->current_command.finished = false;
    connection->current_command.response_index = 0;
    connection->current_command.sent_title = false;
    return TRANSACTION;
}

stm_states transaction_retr(connection_data connection) {
    printf("TRANSACTION RETR\n");
    return TRANSACTION;
}

stm_states transaction_dele(connection_data connection) {
    connection->current_command.finished = false;
    return TRANSACTION;
}

stm_states transaction_noop(connection_data connection) {
    connection->current_command.finished = false;
    return TRANSACTION;
}

stm_states transaction_rset(connection_data connection) {
    connection->current_command.finished = false;
    size_t maildir_size = 0;
    for (int i = 0; i < connection->current_session.mail_count; i++) {
        connection->current_session.mails[i].deleted = false;
        maildir_size += connection->current_session.mails[i].size;
    }
    connection->current_session.maildir_size = maildir_size;
    return TRANSACTION;
}

stm_states transaction_capa(connection_data connection) {
    connection->current_command.finished = false;
    return TRANSACTION;
}

stm_states transaction_quit(connection_data connection) {
    printf("TRANSACTION QUIT\n");
    return QUIT;
}

// -------- WRITE HANDLERS -------

stm_states write_authorization_user(connection_data connection, char * destination, size_t * available_space) {
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

stm_states write_authorization_pass(connection_data connection, char * destination, size_t * available_space) {
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

stm_states write_authorization_capa(connection_data connection, char * destination, size_t * available_space) {
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

stm_states write_authorization_quit(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_stat(connection_data connection, char * destination, size_t * available_space) {
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

stm_states write_transaction_list(connection_data connection, char * destination, size_t * available_space) {
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

stm_states write_transaction_retr(connection_data connection, char * destination, size_t * available_space) {
    char * message = "+OK Message follows"; //Podemos poner este mensaje o poner el size en octetos
    size_t message_length = strlen(message);
    char * error_message = "-ERR No such message";
    size_t error_message_length = strlen(error_message);

    char * end;
    long argument = strtol(connection->current_command.argument_1, &end, 10);
    if (end[0] != '\0' || argument - 1 >= connection->current_session.mail_count || connection->current_session.mails[argument-1].deleted == true ) { //ToDo chequiar q ande
        if (error_message_length > *available_space - END_LINE_LENGTH) {
            return TRANSACTION;
        }
        strncpy(destination, error_message, error_message_length);
        strncpy(destination + error_message_length, END_LINE, END_LINE_LENGTH);
        *available_space = error_message_length + END_LINE_LENGTH;
        connection->current_command.finished = true;
        return TRANSACTION;
    }

    if (message_length > *available_space - END_LINE_LENGTH) {
        return TRANSACTION;
    }
    strncpy(destination, message, message_length);
    strncpy(destination + message_length, END_LINE, END_LINE_LENGTH);
    *available_space = message_length + END_LINE_LENGTH;



    connection->current_command.finished = true;
    return TRANSACTION;
}

stm_states write_transaction_dele(connection_data connection, char * destination, size_t * available_space) {
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

stm_states write_transaction_noop(connection_data connection, char * destination, size_t * available_space) {
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

stm_states write_transaction_rset(connection_data connection, char * destination, size_t * available_space) {
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

stm_states write_transaction_capa(connection_data connection, char * destination, size_t * available_space) {
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

stm_states write_transaction_quit(connection_data connection, char * destination, size_t * available_space) {

}
