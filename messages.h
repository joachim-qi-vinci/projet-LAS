#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#define SERVER_IP "127.0.0.1" /* localhost */
#define MAX_PSEUDO 256

typedef enum
{
  INSCRIPTION_REQUEST = 1,
  INSCRIPTION_OK = 200,
  INSCRIPTION_KO = 400,
  PARTIE_LANCEE = 201,
  NOUVELLE_TUILE = 202,
  DEMANDER_SCORE = 210,
  NOTER_SCORE = 211,
  FIN_DE_PARTIE = 212,
  PARTIE_ANNULEE = 300,
} Code;

/* struct message used between server and client */
typedef struct StructMessage
{
  char messageText[MAX_PSEUDO];
  int code;
} StructMessage;

typedef struct Player
{
    int score;
    char pseudo[MAX_PSEUDO];
    int sockfd;
    int pipefdServeur[2];
    int pipefdClient[2];
} Player;

#endif
