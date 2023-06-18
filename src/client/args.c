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
            "Turtle Client v1.0\n"
            "Implementa Turtle Protocol v1\n"
            "ITBA - 72.07 Protocolos de Comunicación 20231Q -- Grupo 4\n"
            "Alejo Flores Lucey | Andrés Carro Wetzel | Nehuén Gabriel Llanos\n\n");
    exit(0);
}

static void
usage(const char * progname) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n"
        "\n"
        "   --help\n"
        "   -h                        Help message.\n\n"
        "   --token <token>\n"
        "   -t <token>                Authentication token for client.\n\n"
        "   --port <server port>\n"
        "   -p <server port>          POP3 server port connection.\n\n"
        "   --directory <maildir>\n"
        "   -d <maildir>              Path of the directory where all the users with their emails will be located.\n\n"
        "   --add-user <user>:<password>\n"
        "   -u <user>:<password>      Username and password of a user that can use the POP3 server. Up to 10.\n\n"
        "   --change-password <user>:<password>\n"
        "   -c <user>:<password>      Change password for the specified user.\n\n"
        "   --remove-user <user>\n"
        "   -r <user>                 Delete user from the POP3 server.\n\n"
        "   --list-users\n"
        "   -l                        List the users of the POP3 server.\n\n"
        "   --statistics\n"
        "   -s                        Retrieve the statistics of the POP3 server.\n\n"
        "   --max-mails <number>\n"
        "   -m <number>               Change the maximum number of emails.\n\n"
        "   --version\n"
        "   -v                        Print information about the version.\n"
        "\n",
        progname);
    exit(1);
}

void parse_args(const int argc, char **argv, struct args * args) {
    memset(args, 0, sizeof(*args));

    args->port = CLIENT_PORT;

    int c;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { "token",           required_argument, 0, 't' },
            { "port",            required_argument, 0, 'P' },
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

        c = getopt_long(argc, argv, "ht:P:d:u:c:r:lsm:v", long_options, &option_index);
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
            case 'P':
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
        fprintf(stderr, "Token argument must be provided.\n");
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
