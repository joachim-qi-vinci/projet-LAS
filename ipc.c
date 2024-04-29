#include <stdlib.h>
#include "ipc.h"
#include "utils_v1.h"

int shm_id;
int sem_id;

void createScoresTab(int nbr_player) {
    Player* tabPlayer = (Player*) smalloc(nbr_player * sizeof(Player));
    shm_id = sshmget(SHM_KEY, sizeof(tabPlayer), IPC_CREAT | PERM);
    tabPlayer = sshmat(shm_id);
    sem_id = sem_create(SEM_KEY, 1, PERM, 0);
    sshmdt(tabPlayer);
}


void placeScore(Player player, int logical_size) {
    sem_down0(sem_id);
    Player* tabPlayer = sshmat(shm_id);
    tabPlayer[logical_size] = player;
    sem_up0(sem_id);
    sshmdt(tabPlayer);
}

void closeIPC() {
    Player* tabPlayer = sshmat(shm_id);
    free(tabPlayer);
    sshmdelete(shm_id);
    sem_delete(sem_id);
}