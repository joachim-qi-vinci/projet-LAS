#ifndef _NETWORK_H_
#define _NETWORK_H_
#define SERVER_IP "127.0.0.1" 

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"
#include "messages.h"
#include "utils_v1.h"

void disconnect_players(Player *tabPlayers, int nbPlayers);
/**
 * PRE:  serverPort: a valid port number
 * POST: on success, binds a socket to 0.0.0.0:serverPort and listens to it ;
 *       on failure, displays error cause and quits the program
 * RES:  return socket file descriptor
 */
int initSocketServer(int port);

int connectToServer(char* serverIP, int serverPort);

#endif