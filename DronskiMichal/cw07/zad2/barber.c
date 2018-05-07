#include "global.h"


void close_all();
void sigint_action(int sig_number, siginfo_t *siginfo, void *foo);
void barber_serve_customer(pid_t client_pid);
pid_t *barber_queue_init(int size);
const char b_name[] = "qwertyui";

int shared_memory_desc;
void *shared_mem_address;
sem_t *barber_sem, *queue_sem, *chair_sem;

int main(){
    atexit(close_all);

    int size = 10;

    struct sigaction int_action;
    memset(&int_action, 0, sizeof(int_action));
    int_action.sa_sigaction = &sigint_action;
    int_action.sa_flags = SA_SIGINFO;
    check_error(sigaction(SIGINT, &int_action, NULL), -1);

    pid_t *barber_queue = barber_queue_init(size);

    pid_t client_on_chair;
    pid_t next_client;
    printf("%zu - Barber is sleepeing\n", get_time());

    while(1){
        take_semaphore(barber_sem);
        take_semaphore(queue_sem);
        client_on_chair = barber_queue[CLIENT_ON_CHAIR_INDEX];
        give_semaphore(queue_sem);

        barber_serve_customer(client_on_chair);

        while(1){
            take_semaphore(queue_sem);
            print_barber_queue(barber_queue);
            next_client = barber_queue_get(barber_queue);

            if(next_client == -1){
                printf("Queue is empty, barber fall asleep\n");
                take_semaphore(barber_sem);
                give_semaphore(queue_sem);
                break;
            }
            else {
                sit_on_chair(barber_queue ,next_client);
                client_on_chair = next_client;
                barber_serve_customer(client_on_chair);
                give_semaphore(queue_sem);
            }
        }
    }
}

void sigint_action(int sig_number, siginfo_t *siginfo, void *foo){
    printf("Barber received SIGINT\n");
    exit(EXIT_SUCCESS);
}

pid_t *barber_queue_init(int size){

    barber_sem = sem_open(BARBER_SEM_NAME, O_RDWR | O_CREAT | O_EXCL , 0666, 0);
    if(barber_sem == SEM_FAILED){
        printf("ERROR\n");
    }
    queue_sem = sem_open(QUEUE_SEM_NAME, O_RDWR | O_CREAT | O_EXCL , 0666, 1);
    if(queue_sem == SEM_FAILED){
        printf("ERROR2\n");
    }
    chair_sem = sem_open(CHAIR_SEM_NAME, O_RDWR | O_CREAT | O_EXCL, 0666, 0);
    shared_memory_desc = shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT, 0666);

    check_error(ftruncate(shared_memory_desc, size * sizeof(pid_t) + 3), -1);

    shared_mem_address = mmap(0, sizeof(pid_t *), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_desc, 0);

    if(shared_mem_address == MAP_FAILED) {
        perror("");
        exit(EXIT_FAILURE);
    }

    queue_size = size;
    pid_t *barber_queue = (pid_t *) shared_mem_address;
    barber_queue[CLIENTS_COUNT_INDEX] = 0;
    barber_queue[CLIENT_ON_CHAIR_INDEX] = -1;
    barber_queue[MAX_SIZE_INDEX] = size;

    return barber_queue;
}

void close_all(){
    check_error(sem_close(barber_sem), -1);
    check_error(sem_close(queue_sem), -1);
    check_error(sem_close(chair_sem), -1);
    sem_unlink(BARBER_SEM_NAME);
    sem_unlink(QUEUE_SEM_NAME);
    sem_unlink(CHAIR_SEM_NAME);
    munmap(shared_mem_address, queue_size * sizeof(pid_t) + 3);
    shm_unlink(SHARED_MEMORY_NAME);
}

void barber_serve_customer(pid_t client_pid){
    printf("%zu - Barber is making haircut for %d\n", get_time(), client_pid);
    printf("%zu - Barber finished making haircut for %d\n", get_time(), client_pid);
    give_semaphore(chair_sem);
}
