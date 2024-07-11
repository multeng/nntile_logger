#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void websocket_connect();
extern int client_socket;
void websocket_disconnect();

#ifdef __cplusplus
}
#endif

#endif