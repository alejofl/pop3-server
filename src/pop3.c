#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include "pop3.h"

static void print(struct selector_key * key);

void accept_pop_connection(struct selector_key * key) {
    struct sockaddr client_address;
    socklen_t client_address_length = sizeof(client_address);

    int new_socket_fd = accept(key->fd, &client_address, &client_address_length);

    const struct fd_handler handler = {
            .handle_read       = print,
            .handle_write      = NULL,
            .handle_close      = NULL, // nada que liberar
    };

    selector_register(key->s, new_socket_fd, &handler, OP_READ, NULL);
}

static void print(struct selector_key * key) {
    char buf[255];
    int n = read(key->fd, buf, 255);
    buf[n] = 0;
    printf("Nehui compro entradas - %s", buf);
}