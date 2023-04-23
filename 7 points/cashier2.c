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

    cashier_1 = sem_open(SEM_NAME_1, O_CREAT, 0600, 0);
    cashier_2 = sem_open(SEM_NAME_2, O_CREAT, 0600, 0);
    queue = sem_open(SEM_NAME_3, O_CREAT, 0600, 1);
    for_1 = sem_open(SEM_NAME_4, O_CREAT, 0600, 0);
    for_2 = sem_open(SEM_NAME_5, O_CREAT, 0600, 0);
    start = sem_open(SEM_NAME_5, O_CREAT, 0600, 0);

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

    sem_wait(start); // ждем сообщения о прибывающих покупателях

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

    return 0;
}