#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SHM_KEY 2007
#define SEM_KEY 2007

union semnun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main(void)
{
    int shm_id, sem_id;
    int *shm_buf;
    struct sembuf sb[2];
    union semnun arg;
    unsigned short sem_vals[2];

    shm_id = shmget(SHM_KEY, sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
    if (shm_id == -1) {
        fprintf(stderr, "shmget() error\n");
        return 1;
    }

    sem_id = semget(SEM_KEY, 2, IPC_CREAT | IPC_EXCL | 0600);
    if (sem_id == -1) {
        fprintf(stderr, "semget() error\n");
        return 1;
    }

    sem_vals[0] = 1;
    sem_vals[1] = 0;
    arg.array = sem_vals;
    if (semctl(sem_id, 0, SETALL, arg) == -1) {
        fprintf(stderr, "semctl() error\n");
        return 1;
    }

    shm_buf = (int *)shmat(shm_id, NULL, 0);
    if (shm_buf == (int *)-1) {
        fprintf(stderr, "shmat() error\n");
        return 1;
    }

    printf("[Owner] Введите число: ");
    scanf("%d", shm_buf);

    while (1) {
        sb[0].sem_num = 0;
        sb[0].sem_op = -1;
        sb[0].sem_flg = SEM_UNDO;
        if (semop(sem_id, sb, 1) == -1) {
            fprintf(stderr, "semop() wait error\n");
            break;
        }

        if (*shm_buf <= 0) {
            printf("[Owner] end: %d\n", *shm_buf);
            sb[0].sem_num = 1;
            sb[0].sem_op = 1;
            semop(sem_id, sb, 1);
            break;
        }

        (*shm_buf)--;
        printf("[Owner] -> %d\n", *shm_buf);
        fflush(stdout);

        sb[0].sem_num = 1;
        sb[0].sem_op = 1;
        semop(sem_id, sb, 1);

        sleep(1);
    }

    sleep(3);
    shmdt(shm_buf);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID, arg);
    return 0;
}