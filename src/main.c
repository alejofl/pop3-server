#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <sys/socket.h>  // socket
#include <netinet/in.h>

#include "constants.h"
#include "pop3.h"
#include "selector.h"

static bool terminated = false;

static void sigterm_handler(const int signal) {
    printf("Signal %d received, cleaning up and exiting\n", signal);
    terminated = true;
}

static int setup_ipv4_socket(unsigned long port) {
    // Storage for socket address
    struct sockaddr_in address;
    socklen_t address_length = sizeof(address);

    memset(&address, 0, address_length);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server < 0) {
        return -1;
    }

    // man 7 ip
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(server, (struct sockaddr *) &address, address_length) < 0) {
        return -1;
    }

    if (listen(server, MAX_QUEUED_CONNECTIONS) < 0) {
        return -1;
    }

    return 0;
}

static int setup_ipv6_socket(unsigned long port) {
    // Storage for socket address
//    struct sockaddr_in6 address;
//    socklen_t address_length = sizeof(address);
//
//    memset(&address, 0, address_length);
//    address.sin6_family = AF_INET6;
//    address.sin6_addr = in6addr_any;
//    address.sin6_port = htons(port);
//
//    int server = socket(IPV6_V6ONLY, SOCK_STREAM, IPPROTO_TCP);
//
//    if (server < 0) {
//        return -1;
//    }
//
//    // man 7 ip
//    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
//
//    if (bind(server, (struct sockaddr *) &address, address_length) < 0) {
//        return -1;
//    }
//
//    if (listen(server, MAX_QUEUED_CONNECTIONS) < 0) {
//        return -1;
//    }

    return 0;
}

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    close(STDIN_FILENO);
    char * error_message = NULL;

    unsigned long port = 62511;

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

    int ipv4_socket;
    int ipv6_socket = -1;
    selector_status ss = SELECTOR_SUCCESS;
    fd_selector selector = NULL;

    // Initialize the server socket
    ipv4_socket = setup_ipv4_socket(port);
    if (ipv4_socket < 0) {
        error_message = "Failed to initialize IPv4 server socket";
        goto finally;
    }
//    ipv6_socket = setup_ipv6_socket(port);
//    if (ipv6_socket < 0) {
//        error_message = "Failed to initialize IPv6 server socket";
//        goto finally;
//    }

    // Initialize selector
    struct selector_init init_data = {
            .signal = SIGALRM,
            .select_timeout = { .tv_sec = 100, .tv_nsec = 0 }
    };
    ss = selector_init(&init_data);
    if (ss != SELECTOR_SUCCESS) {
        error_message = "Failed to initialize selector";
        goto finally;
    }

    selector = selector_new(MAX_CONCURRENT_CONNECTIONS);
    if (selector == NULL) {
        error_message = "Failed to create selector";
        goto finally;
    }

    // Register the server socket to the selector
    fd_handler server_handler = {
            .handle_read = accept_pop_connection,
            .handle_write = NULL,
            .handle_block = NULL,
            .handle_close = NULL
    };

    ss = selector_register(selector, ipv4_socket, &server_handler, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        error_message = "Failed to register IPv4 server socket in selector";
        goto finally;
    }
//    ss = selector_register(selector, ipv6_socket, &server_handler, OP_READ, NULL);
//    if (ss != SELECTOR_SUCCESS) {
//        error_message = "Failed to register IPv6 server socket in selector";
//        goto finally;
//    }

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

        if (ipv4_socket >= 0) {
            close(ipv4_socket);
        }
//        if (ipv6_socket >= 0) {
//            close(ipv6_socket);
//        }

    return ret;
}