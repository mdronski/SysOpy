#include <wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include "global.h"

int shared_mem_id = -1;
void *shared_mem_address;
int cut_counter = 0;
int cut_required = 3; //////////////////
sem_t *barber_sem, *queue_sem, *chair_sem;
int shared_memory_desc;

void sigint_action(int sig_number, siginfo_t *siginfo, void *foo);
pid_t *configure_shared_memory();
void close_all();
void go_to_barber();
void client_factory(int client_count);
int try_to_get_haircut();

int main(int argc, char *argv[]){
    atexit(close_all);

    int client_count = atoi(argv[1]);
    cut_required = atoi(argv[2]);

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = &sigint_action;
    act.sa_flags = SA_SIGINFO;
    check_error(sigaction(SIGINT, &act, NULL), -1);
    configure_shared_memory();

    client_factory(client_count);
    return 0;

}

pid_t *configure_shared_memory(){
    barber_sem = sem_open(BARBER_SEM_NAME, O_RDWR);
    if(barber_sem == SEM_FAILED){
        printf("ERROR\n");
    }
    queue_sem = sem_open(QUEUE_SEM_NAME, O_RDWR );

    chair_sem = sem_open(CHAIR_SEM_NAME, O_RDWR );

    shared_memory_desc = shm_open(SHARED_MEMORY_NAME, O_RDWR , 0666);

    shared_mem_address = mmap(0,  sizeof(pid_t *), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_desc, 0);
    if(shared_mem_address == MAP_FAILED) {
        perror("");
        exit(EXIT_FAILURE);
    }

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
    check_error(munmap(shared_mem_address, queue_size * sizeof(pid_t) + 3), -1);
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
            take_semaphore(chair_sem);
            ++cut_counter;
            printf("%zu - %d is having hair cut.\n", get_time(), getpid());
        }
    }
}



int try_to_get_haircut() {
    take_semaphore(queue_sem);
    int in_barber_queue;
    int barber_sem_val;
    check_error(sem_getvalue(barber_sem, &barber_sem_val), -1);
    check_error(barber_sem_val,-1);

    if(barber_sem_val == 0) {
        printf("%zu - %d: Waking up barber.\n", get_time(), getpid());
        give_semaphore(barber_sem);
        give_semaphore(barber_sem);
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

    give_semaphore(queue_sem);
    return in_barber_queue;
}













