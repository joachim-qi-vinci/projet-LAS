#ifndef _SERVER_H_
#define _SERVER_H_


#include "messages.h"

#define MAX_PLAYERS 15
#define BACKLOG 5
#define TIME_INSCRIPTION 30



typedef struct Player
{
    int score;
    char pseudo[MAX_PSEUDO];
    int sockfd;
} Player;

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


#endif;