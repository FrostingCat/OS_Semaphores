#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#define SEM_NAME_1 "/mysem1"
#define SEM_NAME_2 "/mysem2"
#define SEM_NAME_3 "/mysem3"
#define SEM_NAME_4 "/mysem4"
#define SEM_NAME_5 "/mysem5"
#define SEM_NAME_6 "/mysem6"
#define SHM_NAME "/my_shared_memory"

sem_t *cashier_1, *cashier_2, *queue, *for_1, *for_2, *start;
int customers_kol;
int* count_array;
int shm_fd;

void my_handler(int nsig) {

    sem_close(cashier_1);
    sem_close(cashier_2);
    sem_close(queue);
    sem_close(for_1);
    sem_close(for_2);
    sem_close(start);
    sem_unlink(SEM_NAME_1);
    sem_unlink(SEM_NAME_2);
    sem_unlink(SEM_NAME_3);
    sem_unlink(SEM_NAME_4);
    sem_unlink(SEM_NAME_5);
    sem_unlink(SEM_NAME_6);
    munmap(count_array, 4);
    close(shm_fd);
    shm_unlink(SHM_NAME);

    exit(0);
}

int main(int argc, char* argv[]) {
    
    signal(SIGINT, my_handler);

    int customers_in_queue = 0;
    char *p;
    long customers_kol = strtol(argv[1], &p, 10);

    printf("The shop opens now. After the dark night full of dangers people ran out of all food. They are going to the shop. \n\nToday we are waiting for %ld customers.\n\n", customers_kol);

    cashier_1 = sem_open(SEM_NAME_1, O_CREAT, 0600, 0);
    cashier_2 = sem_open(SEM_NAME_2, O_CREAT, 0600, 0);
    queue = sem_open(SEM_NAME_3, O_CREAT, 0600, 1);
    for_1 = sem_open(SEM_NAME_4, O_CREAT, 0600, 0);
    for_2 = sem_open(SEM_NAME_5, O_CREAT, 0600, 0);
    start = sem_open(SEM_NAME_5, O_CREAT, 0600, 0);

    // create memory for clients left
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0644);
    if (ftruncate(shm_fd, 4) == -1) {
        perror("ftruncate");
        exit(-1);
    }
    count_array = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    count_array[0] = customers_kol; // keep the number of customers left
    sem_post(start);
    sem_post(start);

    for (int i = 1; i <= customers_kol; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("Can\'t fork customer");
            exit(-1);
        } else if (pid == 0) {

            srand(time(NULL) * i);
            sleep(rand() % 3 + (i / 2));

            sem_wait(queue); // semaphore taking a queue

            fflush(stdout);
            printf("Customer %d is in the queue\n", i);
            customers_in_queue++;

            sem_post(queue); // semaphore for releasing a queue

            int random_cashier;
            if (customers_in_queue == 1) {
                random_cashier = rand() % 2;
                if (random_cashier == 0) {
                    sem_post(cashier_1);
                } else {
                    sem_post(cashier_2);
                }
            }
            if (random_cashier == 0) {
                sem_wait(for_1);
            } else {
                sem_wait(for_2);
            }

            fflush(stdout);
            printf("Customer %d is being served by cashier %d\n", i, random_cashier + 1);
            sleep(2); // service time

            sem_wait(queue); // semaphore taking a queue
            fflush(stdout);
            printf("Customer %d is leaving the supermarket\n", i);
            customers_in_queue--;
            sem_post(queue); // semaphore for releasing a queue

            exit(0);
        }
    }

    for (int i = 1; i <= customers_kol; i++) {
        wait(0); // wait for all customers to finish
    }
    sem_close(cashier_1);
    sem_close(cashier_2);
    sem_close(queue);
    sem_close(for_1);
    sem_close(for_2);
    sem_unlink(SEM_NAME_1);
    sem_unlink(SEM_NAME_2);
    sem_unlink(SEM_NAME_3);
    sem_unlink(SEM_NAME_4);
    sem_unlink(SEM_NAME_5);
    munmap(count_array, 4);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    exit(0);

    return 0;
}