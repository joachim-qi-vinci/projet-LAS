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
            printf("La partie est terminée\n");
            printf("TODO: Afficher le classement\n");
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
        if(msg.code == NOUVELLE_TUILE){
            displayPlateau();
            int tile = atoi(msg.messageText);
            printf("La prochaine tuile est: %d\n", tile);
            int position;
            printf("Entrez la position de la tuile\n:");
            scanf("%d", &position);
            placeTile(position, tile);
            printf("Tuile placée\nEn attente des autres joueurs...\n");
            printf("TODO: Notifier le serveur que la tuile a été placée\n");
        }
        if(msg.code == DEMANDER_SCORE){
            displayPlateau();
            calculateScore();
            printf("TODO: Envoyer le score au serveur\n");
        }
    }
    freePlateau();
    return 0;
}
