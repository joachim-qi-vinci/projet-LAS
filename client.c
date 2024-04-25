#include <stdio.h>
#include <stdlib.h>

#include "network.h"


int main(int argc, char const *argv[])
{
    if(argc != 2){
        printf("Usage: %s [port]\n", argv[0]);
        return(0);
    }
    int SERVER_PORT = atoi(argv[1]);

    printf("Entrez votre pseudo\n:");
    char pseudo[256];
    scanf("%s", pseudo);

    printf("Connection au serveur...\n");
    int sockfd = connectToServer(SERVER_IP, SERVER_PORT);
    printf("%d", sockfd);
    
    return 0;
}
