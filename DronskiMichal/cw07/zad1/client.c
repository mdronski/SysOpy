#include <wait.h>
#include "global.h"

int sem_group_id = -1;
int shared_mem_id = -1;
void *shared_mem_address;
int cut_counter = 0;
int cut_required = 2; //////////////////

void sigint_action(int sig_number, siginfo_t *siginfo, void *foo);
pid_t *configure_shared_memory();
void close_all();
void go_to_barber();
void client_factory(int client_count);
int try_to_get_haircut();

int main(){
    atexit(close_all);

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = &sigint_action;
    act.sa_flags = SA_SIGINFO;
    check_error(sigaction(SIGINT, &act, NULL), -1);
    configure_shared_memory();

    client_factory(5);
    return 0;

}

pid_t *configure_shared_memory(){
    key_t ipc_key = ftok(getenv("HOME"), IPC_GROUP_KEY);

    sem_group_id = semget(ipc_key, 0, 0);
    check_error(sem_group_id, -1);

    shared_mem_id= shmget(ipc_key, 0, 0);
    check_error(shared_mem_id, -1);

    shared_mem_address = shmat(shared_mem_id, NULL, 0);
    if(shared_mem_address == (void *) -1) {
        perror("");
        exit(EXIT_FAILURE);
    }


    pid_t *barber_queue = (pid_t *) shared_mem_address;
    return barber_queue;
}

void sigint_action(int sig_number, siginfo_t *siginfo, void *foo){
    printf("client factory received SIGINT, killing all clients\n");
    killpg(getgid(), SIGTERM);
    exit(EXIT_SUCCESS);
}

void close_all(){
    check_error(shmdt(shared_mem_address), -1);
}

void client_factory(int client_count) {
    int clientsCounter = 0, status;

    for(int i = 0; i < client_count; i++) {
        if(fork() == 0) {
            go_to_barber();
            _exit(EXIT_SUCCESS);
        }
    }

    while(clientsCounter < client_count) {
        wait(&status);
        clientsCounter++;

        if(status == EXIT_SUCCESS) {
           printf("Client exited successfully\n");
        }
        else {
            printf("child error\n");
            exit(EXIT_SUCCESS);
        }
    }
}

void go_to_barber() {
    while(cut_counter < cut_required) {
        int in_barber_queue = try_to_get_haircut();

        if (in_barber_queue == 0) {
            take_semaphore(sem_group_id, CHAIR_SEM_NUMBER);
            ++cut_counter;
            printf("%zu - %d is having hair cut.\n", get_time(), getpid());
        }
    }
}



int try_to_get_haircut() {
    take_semaphore(sem_group_id, QUEUE_SEM_NUMBER);
    int in_barber_queue;
    int barber_sem_val = semctl(sem_group_id, BARBER_SEM_NUMBER, GETVAL);
    check_error(barber_sem_val,-1);

    if(barber_sem_val == 0) {
        printf("%zu - %d: Waking up barber.\n", get_time(), getpid());
        give_semaphore(sem_group_id, BARBER_SEM_NUMBER);
        give_semaphore(sem_group_id, BARBER_SEM_NUMBER);
        sit_on_chair(shared_mem_address, getpid());
        in_barber_queue = 0;
    } else {
        if(barber_queue_put(shared_mem_address, getpid()) == -1) {
            printf("%zu - %d: Barber's queue is full, returning...\n", get_time(), getpid());
            in_barber_queue = -1;
        } else {
            printf("%zu - %d: Barber is busy. Sitting in the waiting room.\n", get_time(), getpid());
            in_barber_queue = 0;
        }
    }

    give_semaphore(sem_group_id, QUEUE_SEM_NUMBER);
    return in_barber_queue;
}













