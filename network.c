#include "utils_v1.h"

int connectToServer(char* serverIP, int serverPort)
{
    int sockfd = ssocket();
    sconnect(serverIP, serverPort, sockfd);
    return sockfd;
}