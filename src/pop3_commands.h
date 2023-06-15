#ifndef POP3_COMMANDS_H
#define POP3_COMMANDS_H

#include "constants.h"
#include "pop3.h"

stm_states authorization_user(struct selector_key * key, connection_data connection);
stm_states authorization_pass(struct selector_key * key, connection_data connection);
stm_states authorization_capa(struct selector_key * key, connection_data connection);
stm_states authorization_quit(struct selector_key * key, connection_data connection);

stm_states transaction_stat(struct selector_key * key, connection_data connection);
stm_states transaction_list(struct selector_key * key, connection_data connection);
stm_states transaction_retr(struct selector_key * key, connection_data connection);
stm_states transaction_dele(struct selector_key * key, connection_data connection);
stm_states transaction_noop(struct selector_key * key, connection_data connection);
stm_states transaction_rset(struct selector_key * key, connection_data connection);
stm_states transaction_capa(struct selector_key * key, connection_data connection);
stm_states transaction_quit(struct selector_key * key, connection_data connection);

stm_states write_authorization_user(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);
stm_states write_authorization_pass(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);
stm_states write_authorization_capa(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);
stm_states write_authorization_quit(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);

stm_states write_transaction_stat(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);
stm_states write_transaction_list(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);
stm_states write_transaction_retr(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);
stm_states write_transaction_dele(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);
stm_states write_transaction_noop(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);
stm_states write_transaction_rset(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);
stm_states write_transaction_capa(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);
stm_states write_transaction_quit(struct selector_key * key, connection_data connection, char * destination, size_t * available_space);

#endif
