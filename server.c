#include "server.h"
#include "network.h"
#include "ipc.h"

/*** globals variables ***/
Player tabPlayers[MAX_PLAYERS];
volatile sig_atomic_t end_inscriptions = 0;
volatile sig_atomic_t end_game = 0;
volatile int nbPLayers = 0;
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
    for (int i = 0; i < nbPLayers; ++i)
    {
        swrite(tabPlayers[i].sockfd, &message, sizeof(message));
    }
    closeIPC();
    disconnect_players(tabPlayers, nbPLayers);
    exit(0);
}

void childHandler(void *param) {
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

    while (1) {
        if (poll(fds, 2, -1) == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < 2; ++i) {
            if (fds[i].revents & POLLIN) {
                ssize_t bytes_read = read(fds[i].fd, &message, sizeof(message));
                if (bytes_read == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                } else if (bytes_read == 0) {
                    // Fin de fichier ou connexion fermée
                    // Traitez ici en fonction de votre logique
                    continue;
                }

                printf("CHILD MESSAGE.CODE = %d\n", message.code);
                if (fds[i].fd == player->pipefdServeur[0]) {
                    // Lecture depuis pipeServeur
                    if (message.code == PARTIE_LANCEE) {
                        printf("Partie lancée pour %s!\n", player->pseudo);
                        sclose(player->pipefdServeur[1]);
                        sclose(player->pipefdClient[0]);
                        swrite(player->sockfd, &message, sizeof(message));
                    }

                    if (message.code == NOUVELLE_TUILE) {
                        printf("Child %s - NOUVELLE_TUILE\n", player->pseudo);
                        swrite(player->sockfd, &message, sizeof(message));
                        printf("Child %s - WRITE\n", player->pseudo);
                    }
                } else if (fds[i].fd == player->sockfd) {
                    // Lecture depuis socketfd
                    if (message.code == TUILE_PLACEE) {
                        printf("Child %s - TUILE_PLACEE\n", player->pseudo);
                        // Traitement du message TUILE_PLACEE
                        swrite(player->pipefdClient[1], &message, sizeof(message));
                        printf("Child %s - WRITE\n", player->pseudo);
                    }
                }
            }
        }
    }
}

/*
void childHandler(void *param)
{
    Player *player = (Player *)param;
    printf("CHILD %s\n", player->pseudo);
    StructMessage message;
    while (sread(player->pipefdServeur[0], &message, sizeof(message)) > 0)
    {
        printf("CHILD MESSAGE.CODE = %d\n", message.code);
        if (message.code == PARTIE_LANCEE)
        {
            printf("Partie lancée pour %s!\n", player->pseudo);
            sclose(player->pipefdServeur[1]);
            sclose(player->pipefdClient[0]);
            swrite(player->sockfd, &message, sizeof(message));
        }

        if (message.code == NOUVELLE_TUILE)
        {
            printf("Child %s - NOUVELLE_TUILE\n", player->pseudo);
            swrite(player->sockfd, &message, sizeof(message));
            printf("Child %s - WRITE\n", player->pseudo);
        }
    }
    
}
*/

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <port> [fichierDeTuiles]\n", argv[0]);
        return (0);
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

    /*sigset_t set;
    ssigemptyset(&set);
    sigaddset(&set, SIGINT);
    ssigprocmask(SIG_BLOCK, &set, NULL);
    */

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

                if (nbPLayers < MAX_PLAYERS)
                {
                    msg.code = INSCRIPTION_OK;
                    nbPLayers++;

                    if (nbPLayers == MAX_PLAYERS)
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
                printf("Nb Inscriptions : %i\n", nbPLayers);
            }
        }
    }

    printf("FIN DES INSCRIPTIONS\n");

    if (nbPLayers < 2)
    {
        printf("PARTIE ANNULEE .. PAS ASSEZ DE JOUEURS\n");
        msg.code = PARTIE_ANNULEE;
        for (i = 0; i < nbPLayers; i++)
        {
            swrite(tabPlayers[i].sockfd, &msg, sizeof(msg));
        }
        disconnect_players(tabPlayers, nbPLayers);
        sclose(sockfd);
        exit(0);
    }
    else
    {
        printf("PARTIE VA DEMARRER ... \n");
        msg.code = PARTIE_LANCEE;
        createScoresTab(nbPLayers);

        for (int i = 0; i < nbPLayers; i++)
        {
            int pipefdServeur[2];
            int pipefdClient[2];

            spipe(pipefdServeur);
            spipe(pipefdClient);

            tabPlayers[i].pipefdServeur = pipefdServeur;
            tabPlayers[i].pipefdClient = pipefdClient;

            fork_and_run1(childHandler, &tabPlayers[i]);

            sclose(pipefdServeur[0]);
            sclose(pipefdClient[1]);

            swrite(pipefdServeur[1], &msg, sizeof(msg));
        }

        for (int i = 0; i < nbPLayers; i++)
        {
            fds[i].fd = tabPlayers[i].sockfd;
            fds[i].events = POLLIN;
        }
        printf("TIRAGE DE LA TUILE\n");
        msg.code = NOUVELLE_TUILE;
        printf("msg.code = %d\n", msg.code);
        for (int i = 0; i < NB_GAME; ++i)
        {
            int tile = drawTile();
            sprintf(msg.messageText, "%d", tile);
            printf("msg.messageText = %s\n", msg.messageText);

            for (int j = 0; j < nbPLayers; ++j)
            {
                swrite(tabPlayers[j].pipefdServeur[1], &msg, sizeof(msg));
            }

            // loop end_game
            while (nbPlayersAlreadyPlayed < nbPLayers)
            {

                ret = poll(fds, nbPLayers, 1000);
                checkNeg(ret, "server poll error");

                if (ret == 0)
                {
                    continue;
                }

                // check player something to read
                for (int k = 0; k < nbPLayers; k++)
                {
                    if (fds[k].revents & POLLIN)
                    {
                        ret = sread(tabPlayers[k].sockfd, &msg, sizeof(msg));
                        // tester si la connexion du client a été fermée: close(sockfd) ==> read renvoie 0
                        // OU utiliser un tableau de booléens fds_invalid[i] pour indiquer
                        // qu'un socket a été traité et ne doit plus l'être (cf. exemple19_avec_poll)
                        // printf("poll detected POLLIN event on client socket %d (%s)... %s", tabPlayers[i].sockfd, tabPlayers[i].pseudo, ret == 0 ? "this socket is closed!\n" : "\n");

                        if (ret != 0)
                        {
                            // tabPlayers[i].shot = msg.code;
                            // TODO: Changement de type (voir erreur lors du make)
                            // printf("%s joue %s\n", tabPlayers[i].pseudo, codeToStr(msg.code));
                            nbPlayersAlreadyPlayed++;
                        }
                    }
                }
            }
        }
    }

    // TODO
    // winner(tabPlayers[0], tabPlayers[1], winnerName);
    printf("GAGNANT : %s\n", winnerName);
    disconnect_players(tabPlayers, nbPLayers);
    closeIPC();
    sclose(sockfd);
    return 0;
}