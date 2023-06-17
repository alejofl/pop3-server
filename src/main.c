#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <sys/socket.h>  // socket
#include <netinet/in.h>

#include <args.h>

#include "server_constants.h"
#include "pop3.h"
#include "selector.h"
#include "client.h"

static bool terminated = false;
struct args args = {0};
struct stats stats = {0};

static void sigterm_handler(const int signal) {
    printf("Signal %d received, cleaning up and exiting\n", signal);
    terminated = true;
}

static int setup_ipv4_tcp_socket(unsigned long port) {
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

    return server;
}

static int setup_ipv6_tcp_socket(unsigned long port) {
    // Storage for socket address
    struct sockaddr_in6 address;
    socklen_t address_length = sizeof(address);

    memset(&address, 0, address_length);
    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(port);

    int server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    if (server < 0) {
        return -1;
    }

    setsockopt(server, IPPROTO_IPV6, IPV6_V6ONLY, &(int){ 1 }, sizeof(int));
    // man 7 ip
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(server, (struct sockaddr *) &address, address_length) < 0) {
        return -1;
    }

    if (listen(server, MAX_QUEUED_CONNECTIONS) < 0) {
        return -1;
    }

    return server;
}

static int setup_ipv4_udp_socket(unsigned long port) {
    // Storage for socket address
    struct sockaddr_in address;
    socklen_t address_length = sizeof(address);

    memset(&address, 0, address_length);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    int client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (client < 0) {
        return -1;
    }

    if (bind(client, (struct sockaddr *) &address, address_length) < 0) {
        return -1;
    }

    return client;
}

static int setup_ipv6_udp_socket(unsigned long port) {
    // Storage for socket address
    struct sockaddr_in6 address;
    socklen_t address_length = sizeof(address);

    memset(&address, 0, address_length);
    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(port);

    int client = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    if (client < 0) {
        return -1;
    }

    setsockopt(client, IPPROTO_IPV6, IPV6_V6ONLY, &(int){ 1 }, sizeof(int));

    if (bind(client, (struct sockaddr *) &address, address_length) < 0) {
        return -1;
    }

    return client;
}

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    close(STDIN_FILENO);
    char * error_message = NULL;

    // Registering signals to de-allocate resources
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    parse_args(argc, argv, &args);

    int ipv4_server_socket;
    int ipv6_server_socket = -1;
    int ipv4_client_socket = -1;
    int ipv6_client_socket = -1;
    selector_status ss = SELECTOR_SUCCESS;
    fd_selector selector = NULL;

    // Initialize the server socket
    ipv4_server_socket = setup_ipv4_tcp_socket(args.server_port);
    if (ipv4_server_socket < 0) {
        error_message = "Failed to initialize IPv4 server socket";
        goto finally;
    }
    ipv6_server_socket = setup_ipv6_tcp_socket(args.server_port);
    if (ipv6_server_socket < 0) {
        error_message = "Failed to initialize IPv6 server socket";
        goto finally;
    }
    ipv4_client_socket = setup_ipv4_udp_socket(args.client_port);
    if (ipv4_client_socket < 0) {
        error_message = "Failed to initialize IPv4 client socket";
        goto finally;
    }
    ipv6_client_socket = setup_ipv6_udp_socket(args.client_port);
    if (ipv6_client_socket < 0) {
        error_message = "Failed to initialize IPv6 client socket";
        goto finally;
    }

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

    fd_handler client_handler = {
            .handle_read = receive_client_directive,
            .handle_write = NULL,
            .handle_block = NULL,
            .handle_close = NULL
    };

    ss = selector_register(selector, ipv4_server_socket, &server_handler, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        error_message = "Failed to register IPv4 server socket in selector";
        goto finally;
    }
    ss = selector_register(selector, ipv6_server_socket, &server_handler, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        error_message = "Failed to register IPv6 server socket in selector";
        goto finally;
    }
    ss = selector_register(selector, ipv4_client_socket, &client_handler, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        error_message = "Failed to register IPv4 server socket in selector";
        goto finally;
    }
    ss = selector_register(selector, ipv6_client_socket, &client_handler, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        error_message = "Failed to register IPv6 server socket in selector";
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

        if (ipv4_server_socket >= 0) {
            close(ipv4_server_socket);
        }
        if (ipv6_server_socket >= 0) {
            close(ipv6_server_socket);
        }

    return ret;
}
