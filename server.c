#include "server.h"
#include "network.h"
#include "ipc.h"
#include  "game.h" 

/*** globals variables ***/
Player tabPlayers[MAX_PLAYERS];
volatile sig_atomic_t end_inscriptions = 0;
volatile sig_atomic_t end_game = 0;
volatile int nbPlayers = 0;
volatile int nbPlayersAlreadyPlayed = 0;

void endServerHandler(int sig)
{
    end_inscriptions = 1;
}

void endGameHandler(int sig)
{
    end_game = 1;
}

void SIGINTHandler(int sig)
{
    StructMessage message;
    message.code = PARTIE_ANNULEE;
    for (int i = 0; i < nbPlayers; ++i)
    {
        swrite(tabPlayers[i].sockfd, &message, sizeof(message));
    }
    closeIPC();
    disconnect_players(tabPlayers, nbPlayers);
    for(int i = 0; i < nbPlayers; i++) {
        free(tabPlayers[i].pipefdServeur);
         free(tabPlayers[i].pipefdClient);
    }
    exit(0);
}

void childHandler(void *param)
{
    Player *player = (Player *)param;
    printf("CHILD %s\n", player->pseudo);
    StructMessage message;
    struct pollfd fds[2];

    // Initialisation de la structure pollfd pour pipeServeur
    fds[0].fd = player->pipefdServeur[0];
    fds[0].events = POLLIN;

    // Initialisation de la structure pollfd pour socketfd
    fds[1].fd = player->sockfd;
    fds[1].events = POLLIN;

    while (1)
    {
        if (poll(fds, 2, -1) == -1)
        {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < 2; ++i)
        {
            if (fds[i].revents & POLLIN)
            {
                ssize_t bytes_read = read(fds[i].fd, &message, sizeof(message));
                if (bytes_read == -1)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                else if (bytes_read == 0)
                {
                    // Fin de fichier ou connexion fermée
                    continue;
                }

                printf("CHILD MESSAGE.CODE = %d\n", message.code);
                if (fds[i].fd == player->pipefdServeur[0])
                {
                    // lecture du code
                    if (message.code == PARTIE_LANCEE)
                    {
                        printf("Partie lancée pour %s!\n", player->pseudo);
                        // Fermer les extrémités des pipes
                        sclose(player->pipefdServeur[1]);
                        sclose(player->pipefdClient[0]);
                        // Envoi du message de démarrage
                        swrite(player->sockfd, &message, sizeof(message));
                    }
                    if (message.code == NOUVELLE_TUILE)
                    {
                        printf("Child %s - NOUVELLE_TUILE\n", player->pseudo);
                        swrite(player->sockfd, &message, sizeof(message));
                        printf("Child %s - WRITE\n", player->pseudo);
                    }

                    if(message.code == DEMANDER_SCORE){
                        printf("Child %s - DEMANDER_SCORE\n", player->pseudo);
                        swrite(player->sockfd, &message, sizeof(message));
                    }

                    if(message.code == FIN_DE_PARTIE){
                        printf("Child %s - FIN_DE_PARTIE\n", player->pseudo);
                        Player* scoresTabSorted = getScoresTab();
                        char classement[MAX_LENGTH_CLASSEMENT];
                        sprintf(classement, "[\n");
                        for (int i = 0; i < nbPlayers; ++i)
                        {   
                            char playerScore[10];
                            char number[10];
                            sprintf(playerScore, "%d", scoresTabSorted[i].score);
                            sprintf(number, "%d", i);
                            strcat(classement, number);
                            strcat(classement, " -> "); // Convertit le score en une chaîne de caractères
                            strcat(classement, scoresTabSorted[i].pseudo); // Ajoute le pseudo du joueur
                            strcat(classement, " : "); // Ajoute un séparateur
                            strcat(classement, playerScore); // Ajoute le score du joueur
                            strcat(classement, "\n");
                        }
                        strcpy(message.messageText, classement);
                        swrite(player->sockfd, &message, sizeof(message));
                    }
                }
                else if (fds[i].fd == player->sockfd)
                {
                    // Lecture depuis socketfd
                    if (message.code == TUILE_PLACEE)
                    {
                        printf("Child %s - TUILE_PLACEE\n", player->pseudo);
                        // Envoie au serveur qu'il faut une nouvelle tuile
                        swrite(player->pipefdClient[1], &message, sizeof(message));
                        printf("Child %s - WRITE\n", player->pseudo);
                    }

                    if(message.code == NOTER_SCORE){
                        printf("Child %s - NOTER_SCORE\n", player->pseudo);
                        swrite(player->pipefdClient[1], &message, sizeof(message));
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <port> [fichierDeTuiles]\n", argv[0]);
        return 0;
    }
    if (argc == 3)
    {
        readAndCreateTilesTab(argv[2]);
    }
    else
    {
        createTilesTab();
    }

    int SERVER_PORT = atoi(argv[1]);
    int sockfd, newsockfd, i;
    StructMessage msg;
    int ret;
    struct pollfd fds[MAX_PLAYERS];
    char winnerName[256];

    ssigaction(SIGALRM, endServerHandler);
    ssigaction(SIGINT, SIGINTHandler);

    sockfd = initSocketServer(SERVER_PORT);
    printf("Le serveur tourne sur le port : %i...\n", SERVER_PORT);

    i = 0;

    // PARTIE INSCRIPTION
    alarm(TIME_INSCRIPTION);

    while (!end_inscriptions)
    {
        /* client trt */
        newsockfd = accept(sockfd, NULL, NULL); // saccept() exit le programme si accept a été interrompu par l'alarme
        if (newsockfd > 0)                      /* no error on accept */
        {
            ret = sread(newsockfd, &msg, sizeof(msg));

            if (msg.code == INSCRIPTION_REQUEST)
            {
                printf("Inscription demandée par le joueur : %s\n", msg.messageText);

                strcpy(tabPlayers[i].pseudo, msg.messageText);
                tabPlayers[i].sockfd = newsockfd;
                i++;

                if (nbPlayers < MAX_PLAYERS)
                {
                    msg.code = INSCRIPTION_OK;
                    nbPlayers++;

                    if (nbPlayers == MAX_PLAYERS)
                    {
                        alarm(0); // cancel alarm
                        end_inscriptions = 1;
                    }
                }
                else
                {
                    msg.code = INSCRIPTION_KO;
                }
                ret = swrite(newsockfd, &msg, sizeof(msg));
                printf("Nb Inscriptions : %i\n", nbPlayers);
            }
        }
    }

    printf("FIN DES INSCRIPTIONS\n");

    if (nbPlayers < 2)
    {
        printf("PARTIE ANNULEE .. PAS ASSEZ DE JOUEURS\n");
        msg.code = PARTIE_ANNULEE;
        for (i = 0; i < nbPlayers; i++)
        {
            swrite(tabPlayers[i].sockfd, &msg, sizeof(msg));
        }
        disconnect_players(tabPlayers, nbPlayers);
        sclose(sockfd);
        exit(0);
    }
    else
    {
        printf("PARTIE VA DEMARRER ... \n");
        msg.code = PARTIE_LANCEE;

        for (int i = 0; i < nbPlayers; i++)
        {
            int pipefdServeur[2];
            int pipefdClient[2];

            tabPlayers[i].pipefdServeur = malloc(2 * sizeof(int));
            tabPlayers[i].pipefdClient = malloc(2 * sizeof(int));

            pipe(pipefdServeur);
            pipe(pipefdClient);

            tabPlayers[i].pipefdServeur[0] = pipefdServeur[0];
            tabPlayers[i].pipefdServeur[1] = pipefdServeur[1];
            tabPlayers[i].pipefdClient[0] = pipefdClient[0];
            tabPlayers[i].pipefdClient[1] = pipefdClient[1];

            fork_and_run1(childHandler, &tabPlayers[i]);

            close(pipefdServeur[0]);
            close(pipefdClient[1]);

            write(pipefdServeur[1], &msg, sizeof(msg));
        }

        for (int i = 0; i < nbPlayers; i++)
        {
            fds[i].fd = tabPlayers[i].sockfd;
            fds[i].events = POLLIN;
        }
        for (int i = 0; i < NB_GAME; i++)
        {
            msg.code = NOUVELLE_TUILE;
            printf("msg.code = %d\n", msg.code);
            int tile = drawTile();
            sprintf(msg.messageText, "%d", tile);
            printf("msg.messageText = %s\n", msg.messageText);

            for (int j = 0; j < nbPlayers; ++j)
            {
                swrite(tabPlayers[j].pipefdServeur[1], &msg, sizeof(msg));
            }

            // Ajouter tous les tubes pipefdClient à l'ensemble des descripteurs surveillés
            for (int i = 0; i < nbPlayers; ++i)
            {
                fds[nbPlayers + i].fd = tabPlayers[i].pipefdClient[0];
                fds[nbPlayers + i].events = POLLIN;
            }

            // Attendre que tous les joueurs aient placé leur tuile
            while (nbPlayersAlreadyPlayed < nbPlayers)
            {
                // Attendre jusqu'à une seconde pour les données à lire sur n'importe quel descripteur surveillé
                ret = poll(fds, nbPlayers * 2, 1000);
                if (ret == -1)
                {
                    perror("poll");
                    exit(EXIT_FAILURE);
                }
                else if (ret == 0)
                {
                    // Aucune donnée à lire sur aucun descripteur surveillé, continuer à la prochaine itération
                    continue;
                }
                else
                {
                    // Parcourir tous les descripteurs surveillés
                    for (int i = 0; i < nbPlayers * 2; ++i)
                    {
                        // Vérifier s'il y a des données à lire sur ce descripteur surveillé
                        if (fds[i].revents & POLLIN)
                        {
                            // Trouver le joueur correspondant au descripteur surveillé
                            int playerIndex = i % nbPlayers;

                            // Lire les données depuis le tube du joueur
                            ret = sread(tabPlayers[playerIndex].pipefdClient[0], &msg, sizeof(msg));
                            if (ret != 0)
                            {
                                if (msg.code == TUILE_PLACEE)
                                {
                                    printf("Nombre qui a déjà joué = %d\n", nbPlayersAlreadyPlayed);
                                    printf("Le joueur %s a placé sa tuile\n", tabPlayers[playerIndex].pseudo);
                                    nbPlayersAlreadyPlayed++;
                                    printf("Nombre qui a déjà joué = %d\n", nbPlayersAlreadyPlayed);
                                }
                                else if (msg.code == DEMANDER_SCORE)
                                {
                                    printf("Le joueur %s demande le score\n", tabPlayers[playerIndex].pseudo);
                                    // Gérer la demande de score ici
                                }
                            }
                        }
                    }
                }
            }
            nbPlayersAlreadyPlayed = 0;
            continue;
        }

        // demande des scores
        createScoresTab(nbPlayers);
        msg.code = DEMANDER_SCORE;
        for (int i = 0; i < nbPlayers; ++i)
        {
            swrite(tabPlayers[i].pipefdServeur[1], &msg, sizeof(msg));
        }


        int scoresReceived = 0;

        for (int i = 0; i < nbPlayers; ++i)
        {
            while(scoresReceived < nbPlayers){
                if(sread(tabPlayers[i].pipefdClient[0], &msg, sizeof(msg)) > 0){
                    if(msg.code == NOTER_SCORE){
                        tabPlayers[i].score = atoi(msg.messageText);
                        placeScore(tabPlayers[i], scoresReceived);
                        scoresReceived++;
                        break;
                    }
                }
            }
        }

        //tri du tableau des scores
        Player* scoresTab = getScoresTab();
        sortTabScores(&scoresTab, nbPlayers);
        sshmdt(scoresTab);
        msg.code = FIN_DE_PARTIE;
        for (int i = 0; i < nbPlayers; ++i)
        {
            swrite(tabPlayers[i].pipefdServeur[1], &msg, sizeof(msg));
        }

        // winner(tabPlayers[0], tabPlayers[1], winnerName);
        printf("GAGNANT : %s\n", winnerName);
        disconnect_players(tabPlayers, nbPlayers);
        closeIPC();
        sclose(sockfd);

        // libérer les pipes
        for(int i = 0; i < nbPlayers; i++) {
            free(tabPlayers[i].pipefdServeur);
            free(tabPlayers[i].pipefdClient);
        }
        return 0;
    }

    return 0;
}
