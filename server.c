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

void endGameHandler(int sig){
    end_game = 1;
}

void SIGINTHandler(int sig) {
    StructMessage message;
    message.code = PARTIE_ANNULEE;
    for (int i = 0; i < nbPLayers; ++i)
    {
        swrite(tabPlayers[i].sockfd, &message, sizeof(message));
    }
    closeIPC();
    exit(0);
}

void childHandler(void *param) {
    Player *player = (Player *) param;

    sclose(player->pipefdServeur[1]);
    sclose(player->pipefdClient[0]);

    StructMessage message;

    sread(player->pipefdServeur[0], &message, sizeof(message));
    swrite(player->sockfd, &message, sizeof(message));

    while(!end_game){
        printf("je suis la !!\n");
        int tile;
        sread(player->pipefdServeur[0], &tile, sizeof(tile));
        printf("%d\n", tile);
        StructMessage tileMessage;
        tileMessage.code = NOUVELLE_TUILE;
        sprintf(tileMessage.messageText, "%d", tile);
        printf("%s", tileMessage.messageText);
        swrite(player->sockfd, &tileMessage, sizeof(tileMessage));
    }
}


int main(int argc, char **argv)
{
    if(argc < 2){
        printf("Usage: %s <port> [fichierDeTuiles]\n", argv[0]);
        return(0);
    }
    if(argc == 3){
        readAndCreateTilesTab(argv[2]);
    }else {
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
    ssigprocmask(SIG_BLOCK, &set, NULL);*/

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
    }

    printf("test");
    for (int i = 0; i < NB_GAME; ++i)
    {
        
        // init poll
        for (i = 0; i < MAX_PLAYERS; i++)
        {
            fds[i].fd = tabPlayers[i].sockfd;
            fds[i].events = POLLIN;
        }

        // loop end_game        
        while (nbPlayersAlreadyPlayed < nbPLayers)
        {
            // poll during 1 second
            ret = poll(fds, MAX_PLAYERS, 1000);
            checkNeg(ret, "server poll error");

            if (ret == 0)
                continue;

            int tile = drawTile();
            printf("%d\n", tile );

            for (int i = 0; i < nbPLayers; ++i)
            {                           
                swrite(tabPlayers[i].pipefdServeur[1], &tile, sizeof(int));
            }

            // check player something to read
            for (i = 0; i < nbPLayers; i++)
            {
                if (fds[i].revents & POLLIN)
                {      
                    ret = sread(tabPlayers[i].sockfd, &msg, sizeof(msg));
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

    

    // TODO
    // winner(tabPlayers[0], tabPlayers[1], winnerName);
    printf("GAGNANT : %s\n", winnerName);
    disconnect_players(tabPlayers, nbPLayers);
    closeIPC();
    sclose(sockfd);
    return 0;
}