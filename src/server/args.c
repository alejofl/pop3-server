#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>
#include "args.h"
#include "server_constants.h"
#include "../constants.h"

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
    fprintf(
            stdout,
            "                                    ___-------___\n"\
            "                                _-~~             ~~-_\n"\
            "                             _-~                    /~-_\n"\
            "          /^\\__/^\\         /~  \\                   /    \\\n"\
            "        /|  O|| O|        /      \\_______________/        \\\n"\
            "       | |___||__|      /       /                \\          \\\n"\
            "       |          \\    /      /                    \\          \\\n"\
            "       |   (_______) /______/                        \\_________ \\\n"\
            "       |         / /         \\                      /            \\\n"\
            "        \\         \\^\\\\         \\                  /               \\     /\n"\
            "          \\         ||           \\______________/      _-_       //\\__//\n"\
            "            \\       ||------_-~~-_ ------------- \\ --/~   ~\\    || __/\n"\
            "              ~-----||====/~     |==================|       |/~~~~~\n"\
            "               (_(__/  ./     /                    \\_\\      \\.\n"\
            "                      (_(___/                         \\_____)_)\n"
            "Turtle POP3 Server v1.0\n"
            "Implementa Turtle Protocol v1\n"
            "ITBA - 72.07 Protocolos de Comunicación 20231Q -- Grupo 4\n"
            "Alejo Flores Lucey | Andrés Carro Wetzel | Nehuén Gabriel Llanos\n\n");
    exit(0);
}

static void
usage(const char * progname) {
    fprintf(stderr,
        "Usage: %s [OPTIONS]...\n"
        "\n"
        "   --help\n"
        "   -h                               This help message.\n\n"
        "   --directory <maildir>\n"
        "   -d <maildir>                     Path to directory where it'll find all users with their mails.\n\n"
        "   --pop3-server-port <pop3 server port>\n"
        "   -p <pop3 server port>            Port for POP3 server connections.\n\n"
        "   --config-server-port <configuration server port>\n"
        "   -P <configuration server port>   Port for configuration client connections\n\n"
        "   --user <user>:<password>\n"
        "   -u <user>:<password>             User and password for a user which can use the POP3 server. Up to 10.\n\n"
        "   --token <token>\n"
        "   -t <token>                       Authentication token for the client.\n\n"
        "   --version\n"
        "   -v                               Prints version information.\n"
        "\n",
        progname);
    exit(1);
}

void parse_args(const int argc, char **argv, struct args * args) {
    memset(args, 0, sizeof(*args));

    args->server_port = SERVER_PORT;
    args->client_port = CLIENT_PORT;
    args->max_mails = INITIAL_MAILS_QTY;

    int c;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { "help",              no_argument,       0, 'h' },
            { "directory",         required_argument, 0, 'd' },
            { "pop3-server-port",  required_argument, 0, 'p' },
            { "config-server-port",required_argument, 0, 'P' },
            { "user",              required_argument, 0, 'u' },
            { "token",             required_argument, 0, 't' },
            { "version",           no_argument,       0, 'v' },
            { 0,                   0,                 0, 0 }
        };

        c = getopt_long(argc, argv, "hd:p:P:u:t:v", long_options, &option_index);
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
            case 't':
                if(strlen(optarg) != CLIENT_TOKEN_LENGTH) {
                    fprintf(stderr, "Token invalid. Must be six alphanumerical characters long: %s.\n", optarg);
                    exit(1);
                }
                strcpy(args->token, optarg);
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
    if (args->token[0] == '\0') {
        fprintf(stderr, "Token argument must be provided.\n");
        exit(1);
    }
    if (args->mail_directory[0] == '\0') {
        fprintf(stderr, "Mail directory argument must be provided.\n");
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
