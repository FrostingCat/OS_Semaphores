#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#define SHM_NAME "/my_shared_memory"

sem_t *cashier_1, *cashier_2, *queue, *for_1, *for_2;
int customers_kol;
int* count_array;
int shm_fd;

void my_handler(int nsig) {

    sem_destroy(cashier_1);
    sem_destroy(cashier_2);
    sem_destroy(queue);
    sem_destroy(for_1);
    sem_destroy(for_2);
    munmap(cashier_1, sizeof(sem_t));
    munmap(cashier_2, sizeof(sem_t));
    munmap(queue, sizeof(sem_t));
    munmap(for_1, sizeof(sem_t));
    munmap(for_2, sizeof(sem_t));
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

    cashier_1 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    cashier_2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    for_1 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    for_2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sem_init(cashier_1, 1, 0);
    sem_init(cashier_2, 1, 0);
    sem_init(queue, 1, 1);
    sem_init(for_1, 1, 0);
    sem_init(for_2, 1, 0);

    // create shared Posix memory
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0644);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, 4) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    count_array = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (count_array == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    count_array[0] = customers_kol; // keep the number of customers left

    // fork two cashiers
    pid_t cashier = fork();

    if (cashier < 0) {
        perror("Can\'t fork cashier 1");
        exit(-1);
    } else if (cashier == 0) { // cashier 1
        while (count_array[0] > 0) {
            sem_wait(cashier_1); // wait for customer
            if (count_array[0] == 0) {
                exit(0);
            }

            fflush(stdout);
            printf("Cashier 1 is serving a customer\n");
            //printf("client : %d\n", count_array[0]);
            count_array[0]--;
            sleep(1);

            if (count_array[0] == 0) {
                sem_post(cashier_2);
            }

            sem_post(for_1); // customer is done
        }
        exit(0);
    } else {
        pid_t cash_cl = fork();

        if (cash_cl == -1) {
            perror("Can\'t fork cashier 2");
            exit(-1);
        } else if (cash_cl == 0) { // cashier 2
            while (count_array[0] > 0) {
                sem_wait(cashier_2); // wait for customer
                if (count_array[0] == 0) {
                    exit(0);
                }

                fflush(stdout);
                printf("Cashier 2 is serving a customer\n");
                //printf("client : %d\n", count_array[0]);
                count_array[0]--;
                sleep(1);

                if (count_array[0] == 0) {
                    sem_post(cashier_1);
                }

                sem_post(for_2); // customer is done
            }
            exit(0);
        } else { // customers
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
            wait(0);
            wait(0);
            for (int i = 1; i <= customers_kol; i++) {
                wait(0); // wait for all customers to finish
            }
            sem_destroy(cashier_1);
            sem_destroy(cashier_2);
            sem_destroy(queue);
            sem_destroy(for_1);
            sem_destroy(for_2);
            munmap(cashier_1, sizeof(sem_t));
            munmap(cashier_2, sizeof(sem_t));
            munmap(queue, sizeof(sem_t));
            munmap(for_1, sizeof(sem_t));
            munmap(for_2, sizeof(sem_t));
            munmap(count_array, 4);
            close(shm_fd);
            shm_unlink(SHM_NAME);
            exit(0);
        }
    }
    return 0;
}