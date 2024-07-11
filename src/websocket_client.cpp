#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "websocket_client.hpp"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5001

int client_socket = -1;

extern "C" void websocket_connect()
{
    struct sockaddr_in server;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
}

extern "C" void websocket_disconnect()
{
    if (client_socket != -1)
    {
        close(client_socket);
        client_socket = -1;
    }
}