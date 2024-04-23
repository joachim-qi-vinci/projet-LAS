#ifndef _IPC_H
#define _IPC_H

#include "messages.h"


#define PERM 0666
#define SEM_KEY 465
#define SHM_KEY 978

int createScoresTab();

void placeScore(Player player);

void closeIPC();

#endif