#include "client.h"
#include "args.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    struct args args = {0};
    parse_args(argc, argv, &args);

    struct sockaddr_in address;
    unsigned int address_length = sizeof(address);

    char request[] = "turtle v1\nS 1 alejos\n\n";
    char response[MAX_CLIENT_RESPONSE] = {0};

    int client_socket;

    // Create socket:
    client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(client_socket < 0){
        printf("Error while connecting to server\n");
        return -1;
    }
    printf("Connected to server successfully\n");

    // Set port and IP:
    address.sin_family = AF_INET;
    address.sin_port = htons(62622);
    address.sin_addr.s_addr = INADDR_ANY;

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        printf("Unable to setup server socket\n");
        return -1;
    }

    // Send the message to server:
    if(sendto(client_socket, request, strlen(request), 0, (struct sockaddr *) &address, address_length) < 0){
        printf("Unable to send message\n");
        return -1;
    }

    // Receive the server's response:
    if(recvfrom(client_socket, response, sizeof(response), 0, (struct sockaddr *) &address, &address_length) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }

    printf("Server's response: %s\n", response);

    // Close the socket:
    close(client_socket);

    return 0;
}
