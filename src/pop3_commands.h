#ifndef POP3_COMMANDS_H
#define POP3_COMMANDS_H

#include "constants.h"
#include "pop3.h"

stm_states authorization_user(connection_data connection);
stm_states authorization_pass(connection_data connection);
stm_states authorization_capa(connection_data connection);
stm_states authorization_quit(connection_data connection);

stm_states transaction_stat(connection_data connection);
stm_states transaction_list(connection_data connection);
stm_states transaction_retr(connection_data connection);
stm_states transaction_dele(connection_data connection);
stm_states transaction_noop(connection_data connection);
stm_states transaction_rset(connection_data connection);
stm_states transaction_top(connection_data connection);
stm_states transaction_capa(connection_data connection);
stm_states transaction_quit(connection_data connection);

#endif
