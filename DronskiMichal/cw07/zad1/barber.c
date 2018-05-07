#include "global.h"

void close_all();
void sigint_action(int sig_number, siginfo_t *siginfo, void *foo);
void barber_serve_customer(pid_t client_pid);
pid_t *barber_queue_init(int size);

int sem_group_id = -1;
int shared_mem_id = -1;
void *shared_mem_address;

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
        take_semaphore(sem_group_id, BARBER_SEM_NUMBER);
        take_semaphore(sem_group_id, QUEUE_SEM_NUMBER);
        client_on_chair = barber_queue[CLIENT_ON_CHAIR_INDEX];
        give_semaphore(sem_group_id, QUEUE_SEM_NUMBER);

        barber_serve_customer(client_on_chair);

        while(1){
            take_semaphore(sem_group_id, QUEUE_SEM_NUMBER);
            print_barber_queue(barber_queue);
            next_client = barber_queue_get(barber_queue);

            if(next_client == -1){
                printf("Queue is empty, barber fall asleep\n");
                take_semaphore(sem_group_id, BARBER_SEM_NUMBER);
                give_semaphore(sem_group_id, QUEUE_SEM_NUMBER);
                break;
            }
            else {
                sit_on_chair(barber_queue ,next_client);
                client_on_chair = next_client;
                barber_serve_customer(client_on_chair);
                give_semaphore(sem_group_id, QUEUE_SEM_NUMBER);
            }
        }
    }
}

void sigint_action(int sig_number, siginfo_t *siginfo, void *foo){
    printf("Barber received SIGINT\n");
    exit(EXIT_SUCCESS);
}

pid_t *barber_queue_init(int size){
    key_t ipc_key = ftok(getenv("HOME"), IPC_GROUP_KEY);

    sem_group_id = semget(ipc_key, SEM_GROUP_SIZE, IPC_CREAT | 0600);
    check_error(sem_group_id, -1);

    check_error(semctl(sem_group_id, BARBER_SEM_NUMBER, SETVAL, 0), -1);
    check_error(semctl(sem_group_id, QUEUE_SEM_NUMBER, SETVAL, 1), -1);
    check_error(semctl(sem_group_id, CHAIR_SEM_NUMBER, SETVAL, 0), -1);

    shared_mem_id = shmget(ipc_key, size * sizeof(pid_t), IPC_CREAT | 0666);
    check_error(shared_mem_id, -1);

    shared_mem_address = shmat(shared_mem_id, NULL, 0);
    if(shared_mem_address == (void *) -1) {
        perror("");
        exit(EXIT_FAILURE);
    }

    pid_t *barber_queue = (pid_t *) shared_mem_address;
    barber_queue[CLIENTS_COUNT_INDEX] = 0;
    barber_queue[CLIENT_ON_CHAIR_INDEX] = -1;
    barber_queue[MAX_SIZE_INDEX] = size;

    return barber_queue;
}

void close_all(){
    shmdt(shared_mem_address);
    shmctl(shared_mem_id, IPC_RMID, NULL);
    semctl(sem_group_id, 0, IPC_RMID);
}

void barber_serve_customer(pid_t client_pid){
    printf("%zu - Barber is making haircut for %d\n", get_time(), client_pid);
    printf("%zu - Barber finished making haircut for %d\n", get_time(), client_pid);
    give_semaphore(sem_group_id ,CHAIR_SEM_NUMBER);
}
