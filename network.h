#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "messages.h"

void disconnect_players(Player *tabPlayers, int nbPlayers);
/**
 * PRE:  serverPort: a valid port number
 * POST: on success, binds a socket to 0.0.0.0:serverPort and listens to it ;
 *       on failure, displays error cause and quits the program
 * RES:  return socket file descriptor
 */
int initSocketServer(int port);

#endif