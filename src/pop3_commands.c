#include <stdio.h>
#include <string.h>
#include <dirent.h>
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
    printf("TRANSACTION LIST\n");
    return TRANSACTION;
}

stm_states transaction_retr(connection_data connection) {
    printf("TRANSACTION RETR\n");
    return TRANSACTION;
}

stm_states transaction_dele(connection_data connection) {
    printf("TRANSACTION DELE\n");
    return TRANSACTION;
}

stm_states transaction_noop(connection_data connection) {
    connection->current_command.finished = false;
    return TRANSACTION;
}

stm_states transaction_rset(connection_data connection) {
    printf("TRANSACTION RSET\n");
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
    size_t error_message_length = strlen(message);

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
    size_t message_length = sprintf(message, "+OK %zu %zu", connection->current_session.mail_count, connection->current_session.maildir_size);

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

}

stm_states write_transaction_retr(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_dele(connection_data connection, char * destination, size_t * available_space) {

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
