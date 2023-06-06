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

#include "selector.h"

static bool done = false;

static void sigterm_handler(const int signal) {
    printf("signal %d, cleaning up and exiting\n", signal);
    done = true;
}

static void print(struct selector_key * key) {
    char buf[255];
    int n = read(key->fd, buf, 255);
    buf[n] = 0;
    printf("Nehui compro entradas - %s", buf);
}

static void accept_connection(struct selector_key * key) {
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

int main(const int argc, const char ** argv) {
    unsigned int port = 1080;

    if (argc == 1) {
        // utilizamos el default
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

    // no tenemos nada que leer de stdin
    close(0);

    const char * err_msg = NULL;
    selector_status ss = SELECTOR_SUCCESS;
    fd_selector selector = NULL;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);

    const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server < 0) {
        err_msg = "unable to create socket";
        goto finally;
    }

    fprintf(stdout, "Listening on TCP port %d\n", port);

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    if (bind(server, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        err_msg = "unable to bind socket";
        goto finally;
    }

    if (listen(server, 20) < 0) {
        err_msg = "unable to listen";
        goto finally;
    }

    // registrar sigterm es util para terminar el programa normalmente.
    // esto ayuda mucho en herramientas como valgrind.
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);

    if (selector_fd_set_nio(server) == -1) {
        err_msg = "getting server socket flags";
        goto finally;
    }
    const struct selector_init conf = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec  = 10,
            .tv_nsec = 0,
        },
    };
    if (selector_init(&conf) != 0) {
        err_msg = "initializing selector";
        goto finally;
    }

    selector = selector_new(1024);
    if (selector == NULL) {
        err_msg = "unable to create selector";
        goto finally;
    }
    const struct fd_handler pop3 = {
        .handle_read       = accept_connection,
        .handle_write      = NULL,
        .handle_close      = NULL, // nada que liberar
    };
    ss = selector_register(selector, server, &pop3, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        err_msg = "registering fd";
        goto finally;
    }

    while (!done) {
        err_msg = NULL;
        ss = selector_select(selector);
        if (ss != SELECTOR_SUCCESS) {
            err_msg = "serving";
            goto finally;
        }
    }

    if (err_msg == NULL) {
        err_msg = "closing";
    }
    int ret = 0;

finally:
    if (ss != SELECTOR_SUCCESS) {
        fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "" : err_msg, ss == SELECTOR_IO ? strerror(errno) : selector_error(ss));
        ret = 2;
    } else if (err_msg) {
        perror(err_msg);
        ret = 1;
    }

    if (selector != NULL) {
        selector_destroy(selector);
    }
    selector_close();

    if (server >= 0) {
        close(server);
    }

    return ret;
}