#ifndef ZAD2_GENERAL_H
#define ZAD2_GENERAL_H

#include <signal.h>
#include <zconf.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/sem.h>
#include <memory.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define BARBER_SEM_NAME "barber_sem"
#define QUEUE_SEM_NAME "/queue_sem"
#define CHAIR_SEM_NAME "/chair_sem"
#define SHARED_MEMORY_NAME "/barber_queue_shared"
#define MAX_SIZE_INDEX 0
#define CLIENTS_COUNT_INDEX 1
#define CLIENT_ON_CHAIR_INDEX 2
#define QUEUE_START_INDEX 3


void check_error(int var, int err_number);
int barber_queue_empty(pid_t *barber_queue);
int barber_queue_full(pid_t *barber_queue);
int barber_queue_put(pid_t *queue, pid_t pid);
pid_t barber_queue_get(pid_t *barber_queue);
void sit_on_chair(pid_t *barber_queue, pid_t pid);
void print_barber_queue(pid_t *barber_queue);
void give_semaphore(sem_t* sem);
void take_semaphore(sem_t* sem);
__time_t get_time();
int queue_size;



#endif //ZAD2_GENERAL_H
