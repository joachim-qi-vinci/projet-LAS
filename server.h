#ifndef _SERVER_H_
#define _SERVER_H_



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "messages.h"
#include "utils_v1.h"
#include "game.h" 

#define MAX_PLAYERS 15
#define BACKLOG 5
#define TIME_INSCRIPTION 30


/**
 * init server
 */
int initSocketServer(int port);

/**
 * stop the inscription process
 */
void endServerHandler(int sig);

/**
 * close connection for all player
 */
void disconnect_players(Player *tabPlayers, int nbPlayers);


#endif