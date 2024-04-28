#include "server.h"
#include "network.h"

/*** globals variables ***/
Player tabPlayers[MAX_PLAYERS];
volatile sig_atomic_t end_inscriptions = 0;

void endServerHandler(int sig)
{
    end_inscriptions = 1;
}


int main(int argc, char **argv)
{
    if(argc < 2){
        printf("Usage: %s <port> [fichierDeTuiles]\n", argv[0]);
        return(0);
    }
    if(argc == 3){
        readAndCreateTilesTab(argv[2]);
    } 
    int SERVER_PORT = atoi(argv[1]);
    int sockfd, newsockfd, i;
    StructMessage msg;
    int ret;
    struct pollfd fds[MAX_PLAYERS];
    char winnerName[256];

    ssigaction(SIGALRM, endServerHandler);

    sockfd = initSocketServer(SERVER_PORT);
    printf("Le serveur tourne sur le port : %i...\n", SERVER_PORT);

    i = 0;
    int nbPLayers = 0;

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
    
        pid_t pid;
        for (i = 0; i < nbPLayers; i++)
        {
            swrite(tabPlayers[i].sockfd, &msg, sizeof(msg));

            pid = sfork();

            if (pid == 0)
            {
                printf("Child process for player %d: PID=%d\n", i + 1, getpid());
                // Here you can put the code that each player process should execute
                printf("Player %d: %s\n", i + 1, tabPlayers[i].pseudo);
            }
        }
        msg.code = PARTIE_LANCEE;
    }

    // GAME PART
    int nbPlayersAlreadyPlayed = 0;

    // init poll
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        fds[i].fd = tabPlayers[i].sockfd;
        fds[i].events = POLLIN;
    }
    // loop game
    while (nbPlayersAlreadyPlayed < MAX_PLAYERS)
    {
        // poll during 1 second
        ret = poll(fds, MAX_PLAYERS, 1000);
        checkNeg(ret, "server poll error");

        if (ret == 0)
            continue;

        // check player something to read
        for (i = 0; i < MAX_PLAYERS; i++)
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

    // TODO
    // winner(tabPlayers[0], tabPlayers[1], winnerName);
    printf("GAGNANT : %s\n", winnerName);
    disconnect_players(tabPlayers, nbPLayers);
    sclose(sockfd);
    return 0;
}