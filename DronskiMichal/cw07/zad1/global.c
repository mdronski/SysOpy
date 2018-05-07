#include <bits/time.h>
#include <time.h>
#include "global.h"

void *shared_mem_address;


void check_error(int var, int err_number){
    if(var == err_number){
        perror("");
        exit(EXIT_FAILURE);
    }
}

int barber_queue_empty(pid_t *barber_queue) {
    return (barber_queue[CLIENTS_COUNT_INDEX] == 0) ? 1 : 0;
}

int barber_queue_full(pid_t *barber_queue) {
    return (barber_queue[CLIENTS_COUNT_INDEX] == barber_queue[MAX_SIZE_INDEX]) ? 1 : 0;
}

int barber_queue_put(pid_t *barber_queue, pid_t pid) {
    if (barber_queue_full(barber_queue)) {
        return -1;
    }

    barber_queue[barber_queue[CLIENTS_COUNT_INDEX] + QUEUE_START_INDEX] = pid;
    barber_queue[CLIENTS_COUNT_INDEX] ++;
    return 0;
}

pid_t barber_queue_get(pid_t *barber_queue) {
    if (barber_queue_empty(barber_queue) == 1) {
        return -1;
    }

    pid_t next_client = barber_queue[QUEUE_START_INDEX];

    for (int i=QUEUE_START_INDEX; i<barber_queue[CLIENTS_COUNT_INDEX] + QUEUE_START_INDEX; i++) {
        barber_queue[i] = barber_queue[i+1];
    }

    barber_queue[CLIENTS_COUNT_INDEX] -= 1;
    return next_client;
}

void sit_on_chair(pid_t *barber_queue, pid_t pid) {
    barber_queue[CLIENT_ON_CHAIR_INDEX] = pid;
}

void print_barber_queue(pid_t *barber_queue){
    if(barber_queue_empty(barber_queue)){
        printf("Barber barber_queue is empty\n");
        return;
    }
    printf("%d sits on the chair\n", barber_queue[CLIENT_ON_CHAIR_INDEX]);

    printf("Barber barber_queue:\n");
    for (int i = QUEUE_START_INDEX; i < barber_queue[CLIENTS_COUNT_INDEX] + QUEUE_START_INDEX; ++i) {
        printf("%d. %d\n",i - QUEUE_START_INDEX + 1, barber_queue[i]);
    }

    printf("Queue max size - %d\n", barber_queue[MAX_SIZE_INDEX]);
    printf("Queue current size - %d\n", barber_queue[CLIENTS_COUNT_INDEX]);

}

void give_semaphore(int sem_group_id, unsigned short sem_number) {
    struct sembuf sem_buffer;
    sem_buffer.sem_num = sem_number;
    sem_buffer.sem_op = 1;
    sem_buffer.sem_flg = 0;
    check_error(semop(sem_group_id, &sem_buffer, 1), -1);
}

void take_semaphore(int sem_group_id, unsigned short sem_number) {
    struct sembuf sem_buffer;
    sem_buffer.sem_num = sem_number;
    sem_buffer.sem_op = -1;
    sem_buffer.sem_flg = 0;
    check_error(semop(sem_group_id, &sem_buffer, 1), -1);
}

__time_t get_time() {
    struct timespec time;
    check_error(clock_gettime(CLOCK_MONOTONIC, &time), -1);

    return time.tv_nsec / 1000;
}

