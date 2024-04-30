#include "server.h"
#include "network.h"
#include "ipc.h"

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
                }
                else if (fds[i].fd == player->sockfd)
                {
                    // Lecture depuis socketfd
                    if (message.code == TUILE_PLACEE)
                    {
                        printf("Child %s - TUILE_PLACEE\n", player->pseudo);
                        message.code = NOUVELLE_TUILE;
                        // Envoie au serveur qu'il faut une nouvelle tuile
                        swrite(player->pipefdClient[1], &message, sizeof(message));
                        printf("Child %s - WRITE\n", player->pseudo);
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
        createScoresTab(nbPlayers);

        for (int i = 0; i < nbPlayers; i++) {
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

            printf("Envoi du démarrage de la partie au pipe %d\n", tabPlayers[i].pipefdServeur[1]);
            write(pipefdServeur[1], &msg, sizeof(msg));
        }

        for (int i = 0; i < nbPlayers; i++)
        {
            fds[i].fd = tabPlayers[i].sockfd;
            fds[i].events = POLLIN;
        }
        printf("TIRAGE DE LA TUILE\n");
        msg.code = NOUVELLE_TUILE;
        printf("msg.code = %d\n", msg.code);
        for (int i = 0; i < NB_GAME; i++)
        {
            int tile = drawTile();
            sprintf(msg.messageText, "%d", tile);
            printf("msg.messageText = %s\n", msg.messageText);

            for (int j = 0; j < nbPlayers; ++j)
            {
                swrite(tabPlayers[j].pipefdServeur[1], &msg, sizeof(msg));
            }

            // Attendre que chaque joueur place sa tuile
            for (int i = 0; i < nbPlayers; ++i)
            {
                while (1)
                {
                    ret = poll(&fds[i], 1, 1000);
                    if (ret == -1)
                    {
                        perror("poll");
                        exit(EXIT_FAILURE);
                    }
                    else if (ret == 0)
                    {
                        printf("Le joueur %s n'a pas encore placé sa tuile\n", tabPlayers[i].pseudo);
                        continue;
                    }
                    else
                    {
                        // Vérifier s'il y a des données à lire sur le tube du joueur
                        if (fds[i].revents & POLLIN)
                        {
                            ret = sread(tabPlayers[i].pipefdClient[0], &msg, sizeof(msg));
                            if (ret != 0)
                            {
                                if (msg.code == TUILE_PLACEE)
                                {
                                    printf("Le joueur %s a placé sa tuile\n", tabPlayers[i].pseudo);
                                    nbPlayersAlreadyPlayed++;
                                }
                                else if (msg.code == DEMANDER_SCORE)
                                {
                                    printf("Le joueur %s demande le score\n", tabPlayers[i].pseudo);
                                    // Gérer la demande de score ici
                                }
                            }
                        }
                    }
                }
            }

            // TODO
            // winner(tabPlayers[0], tabPlayers[1], winnerName);
            printf("GAGNANT : %s\n", winnerName);
            disconnect_players(tabPlayers, nbPlayers);
            closeIPC();
            sclose(sockfd);
            return 0;
        }
    }

    return 0;
}
