#include <stdlib.h>
#include <string.h>
#include "ipc.h"
#include "utils_v1.h"

int shm_id;
int sem_id;
Player* tabPlayer;

void createScoresTab(int nbr_player) {
    tabPlayer = (Player*) smalloc(nbr_player * sizeof(Player));
    shm_id = sshmget(SHM_KEY, sizeof(tabPlayer), IPC_CREAT | PERM);
    Player* tabPlayerIPC = sshmat(shm_id);
    memcpy(tabPlayerIPC, tabPlayer, nbr_player * sizeof(Player));
    sshmdt(tabPlayerIPC);
    sem_id = sem_create(SEM_KEY, 1, PERM, 0);
}


void placeScore(Player player, int logical_size) {
    sem_down0(sem_id);
    Player* tabPlayerIPC = sshmat(shm_id);
    tabPlayerIPC[logical_size] = player;
    sem_up0(sem_id);
    sshmdt(tabPlayerIPC);
}

Player* getFinalScoreTab() {
    Player* tabPlayerIPC = sshmat(shm_id);
    return tabPlayerIPC;
}

void closeIPC() {
    free(tabPlayer);
    sshmdelete(shm_id);
    sem_delete(sem_id);
}