#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>
#include "args.h"

static unsigned short port(const char * s) {
     char * end = 0;
     const long sl = strtol(s, &end, 10);

     if (end == s|| '\0' != *end
        || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
        || sl < 0 || sl > USHRT_MAX) {
         fprintf(stderr, "Port should in in the range of 1-%d. Received %s\n", USHRT_MAX, s);
         exit(1);
     }
     return (unsigned short) sl;
}

static void user(char * s, struct users * user) {
    char * p = strchr(s, ':');
    if (p == NULL) {
        fprintf(stderr, "Password not found\n");
        exit(1);
    } else {
        *p = '\0';
        p++;
        strcpy(user->name, s);
        strcpy(user->pass, p);
    }

}

static void version(void) {
    fprintf(stderr, "POP3 Server v4.20\n"
                    "ITBA - Protocolos de Comunicación 20231Q -- Grupo 4\n"
                    "AQUI VA LA LICENCIA\n");
}

static void
usage(const char * progname) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n"
        "\n"
        "   --help\n"
        "   -h                        Este mensaje de ayuda.\n\n"
        "   --directory <maildir>\n"
        "   -d <maildir>              Path del directorio donde se encotrarán todos los usuarios con sus mails.\n\n"
        "   --pop3-server_port <pop3 server_port>\n"
        "   -p <pop3 server_port>            Puerto entrante para conexiones al servidor POP3.\n\n"
        "   --config-server_port <configuration server_port>\n"
        "   -P <configuration server_port>   Puerto entrante para conexiones de configuración\n\n"
        "   --user\n"
        "   -u <user>:<password>      Usuario y contraseña de usuario que puede usar el servidor POP3. Hasta 10.\n\n"
        "   --version\n"
        "   -v                        Imprime información sobre la versión.\n"
        "\n",
        progname);
    exit(1);
}

void parse_args(const int argc, char **argv, struct args * args) {
    memset(args, 0, sizeof(*args));

    args->server_port = 62511;
    args->client_port = 62622;
    args->max_mails = INITIAL_MAILS_QTY;

    int c;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { "help",       required_argument, 0, 'h' },
            { "directory",  required_argument, 0, 'd' },
            { "pop3-server_port",  required_argument, 0, 'p' },
            { "config-server_port",required_argument, 0, 'P' },
            { "user",       required_argument, 0, 'u' },
            { "version",    required_argument, 0, 'v' },
            { 0,            0,                 0, 0 }
        };

        c = getopt_long(argc, argv, "d:p:P:u:v", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'd':
                strcpy(args->mail_directory, optarg);
                break;
            case 'p':
                args->server_port = port(optarg);
                break;
            case 'P':
                args->client_port = port(optarg);
                break;
            case 'u':
                if (args->users_count >= MAX_USERS) {
                    fprintf(stderr, "Maximum number of command line users reached: %d.\n", MAX_USERS);
                    exit(1);
                } else {
                    user(optarg, &args->users[args->users_count]);
                    args->users_count++;
                }
                break;
            case 'v':
                version();
                break;
            default:
                fprintf(stderr, "Unknown argument %d.\n", c);
                exit(1);
        }

    }
    if (optind < argc) {
        fprintf(stderr, "Argument not accepted: ");
        while (optind < argc) {
            fprintf(stderr, "%s ", argv[optind++]);
        }
        fprintf(stderr, "\n");
        exit(1);
    }
}
