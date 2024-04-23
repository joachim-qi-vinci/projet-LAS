#include "utils_v1.h"
#include "ipc.h"

int shm_id;
int sem_id;


void createScores(int nbr_player) {
    Player[nbr_player] tabPlayer;
    shm_id = sshmget(SHM_KEY,sizeof(tabPlayer), IPC_CREAT | PERM);
    sem_id = sem_create(SEM_KEY, 1, PERM, 0);
}


void placeScore(Player player, int logical_size) {
    Player* tabPlayer = sshmat(shm_id);
    sem_down0(sem_id);
    tabPlayer[logical_size] = player;
    sem_up0(sem_id);
    sshmdt(tabPlayer);
}

void closeIPC() {
    sshmdelete(shm_id);
    sem_delete(sem_id);
}