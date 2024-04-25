<<<<<<< HEAD
#include "utils_v1.h"

int connectToServer(char* serverIP, int serverPort)
{
    int sockfd = ssocket();
    sconnect(serverIP, serverPort, sockfd);
    return sockfd;
}
=======
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#include "network.h"
#include "server.h"


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
>>>>>>> 40dd1e987e50918561db2a0870f12a7900059163
