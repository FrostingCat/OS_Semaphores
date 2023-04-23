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
    char *p;
    long customers_kol = strtol(argv[1], &p, 10);

    printf("The shop opens now. After the dark night full of dangers people ran out of all food. They are going to the shop. \n\nToday we are waiting for %ld customers.\n\n", customers_kol);

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

    count_array[0] = customers_kol; // keep the number of customers left

    semctl(sem_id, cashier1, SETVAL, 0);
    semctl(sem_id, cashier2, SETVAL, 0);
    semctl(sem_id, queue, SETVAL, 1); // for customers queue
    semctl(sem_id, for_1, SETVAL, 0);
    semctl(sem_id, for_2, SETVAL, 0);
    semctl(sem_id, start, SETVAL, 0);
    
    signal_semaphore(start);
    signal_semaphore(start);

    for (int i = 1; i <= customers_kol; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("Can\'t fork customer");
            exit(-1);
        } else if (pid == 0) {

            srand(time(NULL) * i);
            sleep(rand() % 3 + (i / 2));

            wait_semaphore(queue); // semaphore taking a queue

            fflush(stdout);
            printf("Customer %d is in the queue\n", i);
            customers_in_queue++;

            signal_semaphore(queue); // semaphore for releasing a queue

            int random_cashier;
            if (customers_in_queue == 1) {
                random_cashier = rand() % 2;
                signal_semaphore(random_cashier);
            }
            if (random_cashier == 0) {
                wait_semaphore(for_1);
            } else {
                wait_semaphore(for_2);
            }

            fflush(stdout);
            printf("Customer %d is being served by cashier %d\n", i, random_cashier + 1);
            sleep(2); // service time

            wait_semaphore(queue); // semaphore taking a queue
            fflush(stdout);
            printf("Customer %d is leaving the supermarket\n", i);
            customers_in_queue--;
            signal_semaphore(queue); // semaphore for releasing a queue

            exit(0);
        }
    }

    for (int i = 1; i <= customers_kol; i++) {
        wait(0); // wait for all customers to finish
    }
    semctl(sem_id, 0, IPC_RMID, 0);
    semctl(sem_id, 1, IPC_RMID, 0);
    semctl(sem_id, 2, IPC_RMID, 0);
    semctl(sem_id, 3, IPC_RMID, 0);
    semctl(sem_id, 4, IPC_RMID, 0);
    shmdt(count_array);
    exit(0);

    return 0;
}