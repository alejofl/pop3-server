#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "pop3_commands.h"

stm_states authorization_user(connection_data connection){
    printf("AUHTORIZATION USER\n");
    return AUTHORIZATION;
}

stm_states authorization_pass(connection_data connection) {
    printf("AUTHORIZATION PASS\n");
    return AUTHORIZATION;
}

stm_states authorization_capa(connection_data connection) {
    printf("AUHTORIZATION CAPA\n");
    return AUTHORIZATION;
}

stm_states authorization_quit(connection_data connection) {
    printf("AUHTORIZATION QUIT\n");
    return QUIT;
}

stm_states transaction_stat(connection_data connection) {
    printf("TRANSACTION STAT\n");
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
    printf("TRANSACTION NOOP\n");
    return TRANSACTION;
}

stm_states transaction_rset(connection_data connection) {
    printf("TRANSACTION RSET\n");
    return TRANSACTION;
}

stm_states transaction_top(connection_data connection) {
    printf("TRANSACTION TOP\n");
    return TRANSACTION;
}

stm_states transaction_capa(connection_data connection) {
    printf("TRANSACTION CAPA\n");
    return TRANSACTION;
}

stm_states transaction_quit(connection_data connection) {
    printf("TRANSACTION QUIT\n");
    return QUIT;
}

// -------- WRITE HANDLERS -------

// FORRO DE MIERDA
stm_states write_authorization_user(connection_data connection, char * destination, size_t * available_space) {
    char * message = "Authorization USER";
    if (strlen(message) > *available_space - END_LINE_LENGTH) {
        return AUTHORIZATION;
    }
    strncpy(destination, message, strlen(message));
    strncpy(destination + strlen(message), END_LINE, END_LINE_LENGTH);
    connection->current_command.finished = true;
    *available_space = strlen(message) + END_LINE_LENGTH;
    return AUTHORIZATION;
}

stm_states write_authorization_pass(connection_data connection, char * destination, size_t * available_space) {
    char * message = "Authorization PASS";
    if (strlen(message) > *available_space - END_LINE_LENGTH) {
        return AUTHORIZATION;
    }
    strncpy(destination, message, strlen(message));
    strncpy(destination + strlen(message), END_LINE, END_LINE_LENGTH);
    connection->current_command.finished = true;
    *available_space = strlen(message) + END_LINE_LENGTH;
    return TRANSACTION;
}

stm_states write_authorization_capa(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_authorization_quit(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_stat(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_list(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_retr(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_dele(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_noop(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_rset(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_top(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_capa(connection_data connection, char * destination, size_t * available_space) {

}

stm_states write_transaction_quit(connection_data connection, char * destination, size_t * available_space) {

}
