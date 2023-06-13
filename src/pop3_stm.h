#ifndef POP3_STM_H
#define POP3_STM_H

#include <selector.h>
#include "constants.h"

void stm_authorization_arrival(stm_states state, struct selector_key * key);
void stm_authorization_departure(stm_states state, struct selector_key * key);
stm_states stm_authorization_read(struct selector_key * key);
stm_states stm_authorization_write(struct selector_key * key);

void stm_transaction_arrival(stm_states state, struct selector_key * key);
void stm_transaction_departure(stm_states state, struct selector_key * key);
stm_states stm_transaction_read(struct selector_key * key);
stm_states stm_transaction_write(struct selector_key * key);

void stm_update_arrival(stm_states state, struct selector_key * key);
void stm_update_departure(stm_states state, struct selector_key * key);
stm_states stm_update_read(struct selector_key * key);
stm_states stm_update_write(struct selector_key * key);

void stm_error_arrival(stm_states state, struct selector_key * key);
void stm_error_departure(stm_states state, struct selector_key * key);
stm_states stm_error_read(struct selector_key * key);
stm_states stm_error_write(struct selector_key * key);

void stm_quit_arrival(stm_states state, struct selector_key * key);
void stm_quit_departure(stm_states state, struct selector_key * key);
stm_states stm_quit_read(struct selector_key * key);
stm_states stm_quit_write(struct selector_key * key);

#endif