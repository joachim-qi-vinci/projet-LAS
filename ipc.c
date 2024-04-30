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
    int sem_id_sem = sem_get(SEM_KEY, 1);
    int shm_id_ipc = sshmget(SHM_KEY, sizeof(int), 0);
    sem_down0(sem_id_sem);
    Player* tabPlayer = sshmat(shm_id_ipc);
    tabPlayer[logical_size] = player;
    sem_up0(sem_id_sem);
    sshmdt(tabPlayer);
}

void closeIPC() {
    int shm_id_ipc = sshmget(SHM_KEY, sizeof(int), 0);
    int sem_id_sem = sem_get(SEM_KEY, 1);
    Player* tabPlayer = sshmat(shm_id_ipc);
    free(tabPlayer);
    sshmdelete(shm_id_ipc);
    sem_delete(sem_id_sem);
}

Player* getScoresTab(){
    int shm_id_ipc = sshmget(SHM_KEY, sizeof(int), 0);
    Player* tabPlayer = sshmat(shm_id_ipc);
    return tabPlayer;
}