#ifndef POP3_STM_H
#define POP3_STM_H

#include <selector.h>
#include "constants.h"

stm_states stm_authorization_read(struct selector_key * key);
void stm_quit_arrival(stm_states state, struct selector_key * key);

#endif