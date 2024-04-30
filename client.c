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
        while (sread(sockfd, &msg, sizeof(msg)) > 0)
        {
            printf("Code message reçu: %d\n", msg.code);
            if (msg.code == FIN_DE_PARTIE)
            {
                printf("La partie est terminée\n");
                printf("%s", msg.messageText);

                break;
            }
            if (msg.code == PARTIE_LANCEE)
            {
                printf("La partie est lancée\n");
                createPlateau();
                break;
            }

            if (msg.code == PARTIE_ANNULEE)
            {
                printf("La partie est annulée\n");
                exit(1);
            }

            if (msg.code == NOUVELLE_TUILE)
            {
                printf("Nous avons une nouvelle tuile !\n");
                displayPlateau();
                int tile = atoi(msg.messageText);
                printf("La prochaine tuile est: %d\n", tile);
                int position;
                printf("Entrez la position de la tuile\n:");
                scanf("%d", &position);
                placeTile(position, tile);
                printf("Tuile placée\nEn attente des autres joueurs...\n");
                msg.code = TUILE_PLACEE;
                swrite(sockfd, &msg, sizeof(msg));
                printf("La tuile a été placée\n");
            }
            if (msg.code == DEMANDER_SCORE)
            {
                displayPlateau();
                msg.code = NOTER_SCORE;
                sprintf(msg.messageText, "%d", calculateScore());
                write(sockfd, &msg, sizeof(msg));
            }
        }
    }
    freePlateau();
    return 0;
}
