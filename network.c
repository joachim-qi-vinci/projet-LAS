#include "network.h"



void disconnect_players(Player *tabPlayers, int nbPlayers)
{
    for (int i = 0; i < nbPlayers; i++)
        sclose(tabPlayers[i].sockfd);
    return;
}

int initSocketServer(int port)
{
    int sockfd = ssocket();

    /* no socket error */

    // setsockopt -> to avoid Address Already in Use
    // to do before bind !
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    sbind(port, sockfd);

    /* no bind error */
    slisten(sockfd, BACKLOG);

    /* no listen error */
    return sockfd;
}
