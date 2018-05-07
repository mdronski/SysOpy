#ifndef ZAD1_GLOBAL_H
#define ZAD1_GLOBAL_H

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


#define IPC_GROUP_KEY 1
#define SEM_GROUP_SIZE 3
#define BARBER_SEM_NUMBER 0
#define QUEUE_SEM_NUMBER 1
#define CHAIR_SEM_NUMBER 2
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
void give_semaphore(int sem_group_id, unsigned short sem_number);
void take_semaphore(int sem_group_id, unsigned short sem_number);
__time_t get_time();



#endif
