#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <unistd.h>

#define SHM_KEY 2007
#define SEM_KEY 2007

int main(void)
{
    int shm_id, sem_id;
    int *shm_buf;
    struct sembuf sb[1];

    shm_id = shmget(SHM_KEY, sizeof(int), 0600);
    if (shm_id == -1) {
        fprintf(stderr, "shmget() error\n");
        return 1;
    }

    sem_id = semget(SEM_KEY, 2, 0600);
    if (sem_id == -1) {
        fprintf(stderr, "semget() error\n");
        return 1;
    }

    shm_buf = (int *)shmat(shm_id, 0, 0);
    if (shm_buf == (int *)-1) {
        fprintf(stderr, "shmat() error\n");
        return 1;
    }

    printf("[User] Значение = %d\n", *shm_buf);
    fflush(stdout);

    while (1) {
        sb[0].sem_num = 1;
        sb[0].sem_op = -1;
        sb[0].sem_flg = SEM_UNDO;
        if (semop(sem_id, sb, 1) == -1) {
            fprintf(stderr, "semop() wait error\n");
            break;
        }

        if (*shm_buf <= 0) {
            printf("[User] Итог = %d\n", *shm_buf);
            fflush(stdout);
            sb[0].sem_num = 0; 
            sb[0].sem_op = 1;
            semop(sem_id, sb, 1);
            break;
        }
 
        (*shm_buf)--;
        printf("[User] -> %d\n", *shm_buf);
        fflush(stdout);

        sb[0].sem_num = 0;
        sb[0].sem_op = 1;
        if (semop(sem_id, sb, 1) == -1) {
            fprintf(stderr, "semop() signal error\n");
            break;
        }

        sleep(1);
    }

    shmdt(shm_buf);
    return 0;
}