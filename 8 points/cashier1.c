#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/shm.h>

int sem_id;
int customers_kol;
int* count_array;

void wait_semaphore(int sem_num) {
    struct sembuf op = {sem_num, -1, 0};
    semop(sem_id, &op, 1);
}

void signal_semaphore(int sem_num) {
    struct sembuf op = {sem_num, 1, 0};
    semop(sem_id, &op, 1);
}

void my_handler(int nsig) {

    semctl(sem_id, 0, IPC_RMID);
    semctl(sem_id, 1, IPC_RMID);
    semctl(sem_id, 2, IPC_RMID);
    semctl(sem_id, 3, IPC_RMID, 0);
    semctl(sem_id, 4, IPC_RMID, 0);
    shmdt(count_array);

    exit(0);
}

int main(int argc, char* argv[]) {
    
    signal(SIGINT, my_handler);

    int cashier1 = 0, cashier2 = 1, queue = 2, for_1 = 3, for_2 = 4, start = 5;
    int customers_in_queue = 0;

    key_t sem_key;
    char pathname1[]="../3-shr";
    sem_key = ftok(pathname1, 0);

    sem_id = semget(sem_key, 6, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Can\'t create semaphore\n");
        exit(-1);
    }

    const int array_size = 4;
    key_t shm_key;
    int shmid;
    char pathname[]="../3-shr-sem";
    shm_key = ftok(pathname, 0);

    if((shmid = shmget(shm_key, sizeof(int)*array_size,
                       0666 | IPC_CREAT | IPC_EXCL)) < 0)  {
        if((shmid = shmget(shm_key, sizeof(int)*array_size, 0)) < 0) {
            printf("Can\'t connect to shared memory\n");
            exit(-1);
        };
        count_array = (int*)shmat(shmid, NULL, 0);
    } else {
        count_array = (int*)shmat(shmid, NULL, 0);
        for(int i = 0; i < array_size; ++i) {
            count_array[i] = 0;
        }
    }

    semctl(sem_id, cashier1, SETVAL, 0);
    semctl(sem_id, cashier2, SETVAL, 0);
    semctl(sem_id, queue, SETVAL, 1); // for customers queue
    semctl(sem_id, for_1, SETVAL, 0);
    semctl(sem_id, for_2, SETVAL, 0);
    semctl(sem_id, start, SETVAL, 0);

    wait_semaphore(start); // ждем сообщения о прибывающих покупателях
    
    while (count_array[0] > 0) {
        wait_semaphore(cashier1); // wait for customer
        if (count_array[0] == 0) {
            exit(0);
        }

        fflush(stdout);
        printf("Cashier 1 is serving a customer\n");
        //printf("client : %d\n", count_array[0]);
        count_array[0]--;
        sleep(1);

        if (count_array[0] == 0) {
            signal_semaphore(cashier2);
        }

        signal_semaphore(for_1); // customer is done
    }

    semctl(sem_id, 0, IPC_RMID, 0);
    semctl(sem_id, 1, IPC_RMID, 0);
    semctl(sem_id, 2, IPC_RMID, 0);
    semctl(sem_id, 3, IPC_RMID, 0);
    semctl(sem_id, 4, IPC_RMID, 0);
    shmdt(count_array);

    return 0;
}