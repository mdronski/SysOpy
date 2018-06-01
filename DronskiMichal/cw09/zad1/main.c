#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <memory.h>
#include <signal.h>

#define PRINT_SHORTER 1
#define PRINT_EQUALS 2
#define PRINT_LONGER 3

int P;
int K;
int N;
FILE *text_source;
int L = 45;
int compare_type;
int print_type;
int nk;

char **global_buffer;
int buffer_size = 20;
int consumer_position = 0;
int producer_position = 0;
int producers_finished = 0;

pthread_t *producers;
pthread_t *consumers;

pthread_mutex_t consumer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t producer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tmp_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *buffer_mutexes;

pthread_cond_t are_filled_slots = PTHREAD_COND_INITIALIZER;
pthread_mutex_t are_filled_slots_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t slot_consumed = PTHREAD_COND_INITIALIZER;
pthread_mutex_t slot_consumed_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t producers_working = PTHREAD_COND_INITIALIZER;
pthread_mutex_t producers_working_mutex = PTHREAD_MUTEX_INITIALIZER;

void exit_handler(int signo){
    printf("\n\nEXIT PROCEDURE\n\n");
    for (int j = 0; j < P; ++j) {
        pthread_cancel(producers[j]);
    }

    for (int i = 0; i < K; ++i) {
        pthread_cancel(consumers[i]);
    }

    fclose(text_source);
    free(producers);
    free(consumers);
    exit(EXIT_SUCCESS);

}

void check_error(int returned_value, int normal_value, char *error_message){
    if (returned_value != normal_value){
        perror(error_message);
        exit(EXIT_FAILURE);
    }
}

void initialise(){

    global_buffer = malloc(N * sizeof(char*));
    for (int j = 0; j < N; ++j) {
        global_buffer[j] = NULL;
    }

    buffer_mutexes = malloc(N * sizeof(pthread_mutex_t));

    for (int i = 0; i < N; ++i) {
        check_error(pthread_mutex_init(&buffer_mutexes[i], NULL), 0, "mutex initialisation");
    }

    signal(SIGINT, exit_handler);
    signal(SIGALRM, exit_handler);
}

void compare_and_print(int array_index, char *string){
    int string_length = (int) strlen(string);
    switch (compare_type) {
        case PRINT_SHORTER:
            if (string_length < L){
                printf("At index: %d, founded string with length %d < %d\n%s\n",
                       array_index, string_length, L, string);
            }
            break;
        case PRINT_EQUALS:
            if (string_length == L){
                printf("At index: %d, founded string with length %d == %d\n%s\n",
                       array_index, string_length, L, string);
            }
            break;
        case PRINT_LONGER:
            if (string_length > L){
                printf("At index: %d, founded string with length %d > %d\n%s\n",
                       array_index, string_length, L, string);
            }
            break;

        default:break;
    }
    fflush(stdout);
}

void mutex_lock(pthread_mutex_t *mutex){
    check_error(pthread_mutex_lock(mutex), 0, "locking mutex");
}

void mutex_unlock(pthread_mutex_t *mutex){
    check_error(pthread_mutex_unlock(mutex), 0, "unlocking mutex");
}

void *producer(void *args){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    char buffer[512];
    int str_length;


    while (fgets(buffer, 512, text_source) != NULL){

        str_length = (int) strlen(buffer);

        mutex_lock(&producer_mutex);

        while (global_buffer[producer_position] != NULL){
            pthread_cond_wait(&slot_consumed, &slot_consumed_mutex);
        }


        mutex_lock(&buffer_mutexes[producer_position]);

        global_buffer[producer_position] = malloc((str_length + 1) * sizeof(char));

        strcpy(global_buffer[producer_position], buffer);

        pthread_cond_broadcast(&are_filled_slots);

        mutex_unlock(&buffer_mutexes[producer_position]);

        producer_position = (producer_position + 1) % buffer_size;
        mutex_unlock(&producer_mutex);
    }

    producers_finished ++;

}

void *consumer(void *args){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (producers_finished == 0) {

        mutex_lock(&consumer_mutex);

        while (global_buffer[consumer_position] == NULL) {
            pthread_cond_wait(&are_filled_slots, &are_filled_slots_mutex);
        }

        mutex_lock(&buffer_mutexes[consumer_position]);

        compare_and_print(consumer_position, global_buffer[consumer_position]);

        free(global_buffer[consumer_position]);

        global_buffer[consumer_position] = NULL;

        pthread_cond_broadcast(&slot_consumed);

        mutex_unlock(&buffer_mutexes[consumer_position]);

        consumer_position = (consumer_position + 1) % buffer_size;
        mutex_unlock(&consumer_mutex);
        usleep(1000);
    }

}

void init_threads(){

    producers = malloc(P * sizeof(pthread_t *));
    for (int i = 0; i < P; ++i) {
        producers[i]  = (pthread_t) malloc(sizeof(pthread_t));
    }

    consumers = malloc(K * sizeof(pthread_t *));
    for (int i = 0; i < K; ++i) {
        consumers[i]  = (pthread_t) malloc(sizeof(pthread_t));
    }

    for (int i = 0; i < P; ++i) {
        check_error(pthread_create(&producers[i], NULL, &producer, NULL), 0, "producer threads creating");
    }

    for (int i = 0; i < K; ++i) {
        check_error(pthread_create(&consumers[i], NULL, &consumer,  NULL), 0, "consumer threads creating");
    }


}


void wait_for_threads(){
    for (int i = 0; i < P; ++i) {
        check_error(pthread_join(producers[i], NULL), 0, "producer threads join");
    }

    for (int i = 0; i < K; ++i) {
        check_error(pthread_join(consumers[i], NULL), 0, "consumer threads join");
    }

    fclose(text_source);
    free(producers);
    free(consumers);
}

int read_integer(FILE *stream){
    int n;
    fscanf(stream, "%d", &n);
    return n;
}

void configure(){
    FILE *configuration_file = fopen("config.txt", "r");
    P = read_integer(configuration_file);
    K = read_integer(configuration_file);
    N = read_integer(configuration_file);

    char buffer[256];
    fscanf(configuration_file, "%s", buffer);
    text_source = fopen(buffer, "r");
    if (text_source == NULL){
        printf("ERROR\n");
    }
    L = read_integer(configuration_file);
    compare_type = read_integer(configuration_file);
    print_type = read_integer(configuration_file);
    nk = read_integer(configuration_file);

    fclose(configuration_file);

    if (nk > 0){
        alarm((unsigned int) nk);
    }

}

int main() {

    configure();

    initialise();

    init_threads();

    wait_for_threads();

    return 0;
}