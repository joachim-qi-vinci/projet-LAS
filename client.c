#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "messages.h"
#include "game.h"

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s [port]\n", argv[0]);
        return (0);
    }
    int SERVER_PORT = atoi(argv[1]);

    printf("Entrez votre pseudo\n:");
    char pseudo[256];
    scanf("%s", pseudo);
    StructMessage msg;
    msg.code = INSCRIPTION_REQUEST;
    strcpy(msg.messageText, pseudo);

    printf("Connexion au serveur...\n");
    int sockfd = connectToServer(SERVER_IP, SERVER_PORT);

    swrite(sockfd, &msg, sizeof(msg));
    sread(sockfd, &msg, sizeof(msg));
    if (msg.code == INSCRIPTION_OK)
    {
        printf("Inscription réussie\n");
    }
    else
    {
        printf("Inscription échouée\n");
    }

    // LANCER LE DEBUT DE LA PARTIE
    // WHILE PARTIE EN COURS
    while (1)
    {
        sread(sockfd, &msg, sizeof(msg));
        if (msg.code == FIN_DE_PARTIE)
        {
            break;
        }
        if (msg.code == PARTIE_LANCEE)
        {
            printf("La partie est lancée\n");
            createPlateau();
        }

        if(msg.code == PARTIE_ANNULEE){
            printf("La partie est annulée\n");
            break;
        }
    }

    return 0;
}
