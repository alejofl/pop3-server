#include <stdio.h>
#include "pop3_commands.h"

stm_states authorization_user(connection_data connection){
    printf("AUHTORIZATION USER\n");
    return AUTHORIZATION;
}

stm_states authorization_pass(connection_data connection) {
    printf("AUHTORIZATION PASS\n");
    return TRANSACTION;
}

stm_states authorization_capa(connection_data connection) {
    printf("AUHTORIZATION CAPA\n");
    return AUTHORIZATION;
}

stm_states authorization_quit(connection_data connection) {
    printf("AUHTORIZATION QUIT\n");
    return AUTHORIZATION;
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
    return AUTHORIZATION;
}