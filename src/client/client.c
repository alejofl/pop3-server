#include "../constants.h"
#include "args.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static const char command_identifiers[] = {'u', 'c', 'r', 'd', 'm', 's', 'l'};
static const char * command_description[] = {"Add User",
                                             "Change Password",
                                             "Remove User",
                                             "Change Mail Directory",
                                             "Change Maximum Mails",
                                             "Get Server Statistics",
                                             "List Users"};
static const int commands_count = sizeof(command_identifiers);

int main(int argc, char * argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    struct args args = {0};
    parse_args(argc, argv, &args);

    struct sockaddr_in address;
    unsigned int address_length = sizeof(address);

    char request[CLIENT_BUFFER_SIZE];
    char response[CLIENT_BUFFER_SIZE];

    int client_socket;

    // Create socket:
    client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(client_socket < 0){
        printf("Error while connecting to server\n");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(args.port);
    address.sin_addr.s_addr = INADDR_ANY;

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        printf("Unable to setup server socket\n");
        close(client_socket);
        return -1;
    }

    for (size_t i = 0; i < args.commands_length; i++) {
        memset(request, 0, CLIENT_BUFFER_SIZE);
        memset(response, 0, CLIENT_BUFFER_SIZE);

        int command_identifier = -1;
        for (int j = 0; j < commands_count; j++) {
            if (args.commands[i].command == command_identifiers[j]) {
                command_identifier = j;
                break;
            }
        }
        if (command_identifier == -1) {
            printf("Invalid command: %c\n\n", args.commands[i].command);
        }

        sprintf(
            request,
            "turtle v1\n%c %d %s\n%s\n",
            args.commands[i].command - ('a' - 'A'),
            (int) i,
            args.token,
            args.commands[i].command == 'l' || args.commands[i].command == 's' ? "" : args.commands[i].content
        );

        if (sendto(client_socket, request, strlen(request), 0, (struct sockaddr *) &address, address_length) < 0) {
            printf("%s: Unable to send request\n\n", command_description[command_identifier]);
            continue;
        }

        if (recvfrom(client_socket, response, sizeof(response), 0, (struct sockaddr *) &address, &address_length) < 0) {
            printf("%s: Server did not respond\n\n", command_description[command_identifier]);
            continue;
        }

        int status;
        int id;
        char content[CLIENT_CONTENT_LENGTH];
        int k = sscanf(response, "%d %d\n%[^\n]\n", &status, &id, content);

        if (k < 2 || (k == 2 && args.commands[i].command == 's') || (k == 2 && args.commands[i].command == 'l')) {
            if (status == 10) {
                printf("%s: Invalid response\n\n", command_description[command_identifier]);
            }
        }

        if (id != (int) i) {
            printf("%s: Invalid response\n\n", command_description[command_identifier]);
        }

        char * status_message;
        switch (status) {
            case ALL_GOOD:
                status_message = "All good";
                break;
            case NOT_EXISTS:
                status_message = "User does not exist";
                break;
            case ALREADY_EXISTS:
                status_message = "User already exists";
                break;
            case MAXIMUM_REACHED:
                status_message = "Maximum users amount reached";
                break;
            case INVALID_VERSION:
                status_message = "Invalid Turtle Protocol version";
                break;
            case INVALID_AUTHENTICATION:
                status_message = "Invalid authentication token";
                break;
            default:
                status_message = "Oops! Something went wrong";
                break;
        }

        printf("%s: %s\n%s\n\n", command_description[command_identifier], status_message, content[0] == '\0' ? "No content" : content);
    }

    close(client_socket);

    return 0;
}
