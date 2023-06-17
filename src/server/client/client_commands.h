#ifndef CLIENT_COMMANDS_H
#define CLIENT_COMMANDS_H

#include "client.h"

void write_error(char * buffer, struct client_command * command);
void add_user(char * buffer, struct client_command * command);
void change_password(char * buffer, struct client_command * command);
void remove_user(char * buffer, struct client_command * command);
void change_directory(char * buffer, struct client_command * command);
void change_max_mails(char * buffer, struct client_command * command);
void get_statistics(char * buffer, struct client_command * command);
void list_users(char * buffer, struct client_command * command);

#endif
