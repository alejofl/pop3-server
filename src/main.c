#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "constants.h"
#include "pop3.h"
#include "selector.h"

static bool terminated = false;

static void sigterm_handler(const int signal) {
    printf("Signal %d received, cleaning up and exiting\n", signal);
    terminated = true;
}

static int setup_socket(char * ip, unsigned long port) {
    // Storage for socket address
    struct sockaddr_storage * address;
    socklen_t address_length;

    bool is_ipv6 = strchr(ip, ':') != NULL;

    struct sockaddr_in6 sock6;
    struct sockaddr_in sock4;

    if (is_ipv6) {
        memset(&sock6, 0, sizeof(sock6));

        sock6.sin6_family = AF_INET6;
        sock6.sin6_addr = in6addr_any;
        sock6.sin6_port = htons(port);
        if (inet_pton(AF_INET6, ip, &sock6.sin6_addr) != 1) {
            return -1;
        }

        address = (struct sockaddr_storage *) &sock6;
        address_length = sizeof(struct sockaddr_in6);
    } else {
        memset(&sock4, 0, sizeof(sock4));

        sock4.sin_family = AF_INET;
        sock4.sin_addr.s_addr = INADDR_ANY;
        sock4.sin_port = htons(port);

        if (inet_pton(AF_INET, ip, &sock4.sin_addr) != 1) {

            return -1;
        }

        address = (struct sockaddr_storage *) &sock4;
        address_length = sizeof(struct sockaddr_in);
    }

    int server = socket((*address).ss_family, SOCK_STREAM, IPPROTO_TCP);

    if (server < 0) {
        return -1;
    }

    // man 7 ip
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(server, (struct sockaddr *) address, address_length) < 0) {
        printf("%d\n", errno);
        return -1;
    }

    if (listen(server, MAX_QUEUED_CONNECTIONS) < 0) {
        return -1;
    }

    return 0;
}

int main(int argc, char** argv) {
    close(STDIN_FILENO);
    char * error_message = NULL;

    unsigned long port = 62511;
    char * ip = "127.0.0.1";

    // Registering signals to de-allocate resources
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    // TODO Use argc.c provided by El Coda
    if (argc == 1) {

    } else if (argc == 2) {
        char * end = 0;
        const long sl = strtol(argv[1], &end, 10);

        if (end == argv[1] || '\0' != *end
            || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
            || sl < 0 || sl > USHRT_MAX) {
            fprintf(stderr, "port should be an integer: %s\n", argv[1]);
            return 1;
        }
        port = sl;
    } else {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    // TODO END


    // Initialize the server socket
    int server_socket = setup_socket(ip, port);

    if (server_socket < 0) {
        error_message = "Failed to initialize server socket";
        goto finally;
    }

    // Initialize selector
    struct selector_init init_data = {
            .signal = SIGALRM,
            .select_timeout = { .tv_sec = 100, .tv_nsec = 0 }
    };
    selector_status ss = selector_init(&init_data);
    if (ss != SELECTOR_SUCCESS) {
        error_message = "Failed to initialize selector";
        goto finally;
    }

    fd_selector selector = selector_new(MAX_CONCURRENT_CONNECTIONS);
    if (selector == NULL) {
        error_message = "Failed to create selector";
        goto finally;
    }

    // Register the server socket to the selector
    fd_handler server_handler = {
            .handle_read = accept_pop_connection, // TODO
            .handle_write = NULL,
            .handle_block = NULL,
            .handle_close = NULL
    };

    ss = selector_register(selector, server_socket, &server_handler, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        error_message = "Failed to register server socket in selector";
        goto finally;
    }

    // Main server loop
    while (!terminated) {
        ss = selector_select(selector);
        if (ss != SELECTOR_SUCCESS) {
            error_message = "Failed to select";
            goto finally;
        }
    }

    int ret = 0;

    finally:
        if (ss != SELECTOR_SUCCESS) {
            fprintf(stderr, "%s: %s\n", (error_message == NULL) ? "" : error_message, ss == SELECTOR_IO ? strerror(errno) : selector_error(ss));
            ret = 2;
        } else if (error_message) {
            perror(error_message);
            ret = 1;
        }

        if (selector != NULL) {
            selector_destroy(selector);
        }
        selector_close();

        if (server_socket >= 0) {
            close(server_socket);
        }

    return ret;
}