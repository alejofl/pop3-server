#include "client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
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