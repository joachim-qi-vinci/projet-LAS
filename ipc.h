#ifndef _IPC_H
#define _IPC_H

#include "messages.h"
#include "utils_v1.h"


#define PERM 0666
#define SEM_KEY 465
#define SHM_KEY 978

void createScoresTab(int nbr_player);

void placeScore(Player player, int logical_size);

void closeIPC();

#endif