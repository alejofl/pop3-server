#include "client.h"
#include "constants.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

void receive_client_directive(struct selector_key * key) {
    struct sockaddr_in client;
    unsigned int client_length = sizeof(client);

    char read_buffer[CLIENT_BUFFER_SIZE] = {0};
    char write_buffer[CLIENT_BUFFER_SIZE] = {0};

    ssize_t n = recvfrom(key->fd, read_buffer, CLIENT_BUFFER_SIZE, 0, (struct sockaddr *) &client, &client_length);
    if (n <= 0) {
        return;
    }
    read_buffer[n] = 0;

    // TODO Client logic

    if (sendto(key->fd, write_buffer, strlen(write_buffer), 0, (struct sockaddr *) &client, client_length) < 0) {
        return;
    }
}
