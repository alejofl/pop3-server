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

static void add_command(char command, char * content, struct command * slot) {
    if (content != NULL && strlen(content) > CLIENT_CONTENT_LENGTH - 1) {
        fprintf(stderr, "Content for argument %d must be less than %d characters long.\n", command, CLIENT_CONTENT_LENGTH);
        exit(1);
    }
    slot->command = command;
    if (content != NULL) {
        strcpy(slot->content, content);
    }
}

static void version(void) {
    fprintf(stderr, "Turtle Client - Turtle Protocol v1\n"
                    "ITBA - Protocolos de Comunicación 20231Q -- Grupo 4\n");
}

static void
usage(const char * progname) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n"
        "\n"
        "   --help\n"
        "   -h                        Este mensaje de ayuda.\n\n"
        "   --token <token>\n"
        "   -t <token>                Token de autenticación para el cliente.\n\n"
        "   --port <server port>\n"
        "   -p <server port>           Puerto para conexiones al servidor POP3 a administrar.\n\n"
        "   --directory <maildir>\n"
        "   -d <maildir>              Path del directorio donde se encotrarán todos los usuarios con sus mails.\n\n"
        "   --add-user <user>:<password>\n"
        "   -u <user>:<password>      Usuario y contraseña de usuario que puede usar el servidor POP3. Hasta 10.\n\n"
        "   --change-password <user>:<password>\n"
        "   -c <user>:<password>      Cambiar contraseña para el usuario especificado.\n\n"
        "   --remove-user <user>\n"
        "   -r <user>                 Eliminar usuario del servidor POP3.\n\n"
        "   --list-users\n"
        "   -l                        Listar los usuarios del servidor POP3.\n\n"
        "   --statistics\n"
        "   -s                        Obtener las estadísticas del servidor POP3.\n\n"
        "   --max-mails <number>\n"
        "   -m <number>               Cambiar el máximo número de mails.\n\n"
        "   --version\n"
        "   -v                        Imprime información sobre la versión.\n"
        "\n",
        progname);
    exit(1);
}

void parse_args(const int argc, char **argv, struct args * args) {
    memset(args, 0, sizeof(*args));

    args->port = 62622;

    int c;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { "token",           required_argument, 0, 't' },
            { "port",            required_argument, 0, 'p' },
            { "directory",       required_argument, 0, 'd' },
            { "add-user",        required_argument, 0, 'u' },
            { "change-password", required_argument, 0, 'c' },
            { "remove-user",     required_argument, 0, 'r' },
            { "max-mails",       required_argument, 0, 'm' },
            { "help",            no_argument,       0, 'h' },
            { "list-users",      no_argument,       0, 'l' },
            { "statistics",      no_argument,       0, 's' },
            { "version",        no_argument,       0, 'v' },
            { 0,                0,                 0, 0 }
        };

        c = getopt_long(argc, argv, "ht:p:d:u:c:r:lsm:v", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 't':
                if(strlen(optarg) != CLIENT_TOKEN_LENGTH) {
                    fprintf(stderr, "Token invalid. Must be six alphanumerical characters long: %s.\n", optarg);
                    exit(1);
                }
                strcpy(args->token, optarg);
                break;
            case 'p':
                args->port = port(optarg);
                break;
            case 'd':
            case 'u':
            case 'c':
            case 'r':
            case 'm':
                if (args->commands_length >= MAX_COMMANDS) {
                    fprintf(stderr, "Maximum arguments reached: %d.\n", MAX_COMMANDS);
                    exit(1);
                }
                add_command(c, optarg, &args->commands[args->commands_length]);
                args->commands_length++;
                break;
            case 'l':
            case 's':
                if (args->commands_length >= MAX_COMMANDS) {
                    fprintf(stderr, "Maximum arguments reached: %d.\n", MAX_COMMANDS);
                    exit(1);
                }
                add_command(c, NULL, &args->commands[args->commands_length]);
                args->commands_length++;
                break;
            case 'v':
                version();
                break;
            default:
                fprintf(stderr, "Unknown argument %d.\n", c);
                exit(1);
        }
    }
    if (args->token[0] == '\0') {
        fprintf(stderr, "Token argument must be provided.");
        exit(1);
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
