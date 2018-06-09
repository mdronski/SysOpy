#define UNIX_PATH_MAX 108
#define MAX_CLIENTS 20

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <zconf.h>
#include <signal.h>
#include "globals.h"

int web_socket;
int local_socket;
int epoll;
int client_number = 0;
int messages_send_conter = 0;
char* unix_path;

pthread_t pinger;
pthread_t commander;

Client_info clients_info[MAX_CLIENTS];

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void error_exit(char *error_message);
void clean_exit();
void validate_args(char **args);
int initialise_web_socket(int socket_number);
int initialise_local_socket(char *unix_path);
int initialise_epoll_monitor();
void* pinger_task(void* args);
void* commander_task(void* args);
void remove_client(int position);
void handle_message(int socket);
void login_handler(int socket, struct sockaddr *address, socklen_t address_size, Message message);
void sigint_handler(int signo);

int main(int argc, char** argv) {
    atexit(clean_exit);
    signal(SIGINT, sigint_handler);
    srand((unsigned int) time(NULL));

    validate_args(argv);
    int socket = atoi(argv[1]);
    unix_path = argv[2];

    web_socket = initialise_web_socket(socket);
    local_socket = initialise_local_socket(unix_path);
    epoll = initialise_epoll_monitor();

    if(pthread_create(&pinger, NULL, pinger_task, NULL) != 0)
        error_exit("creating pinger thread failure");
    if(pthread_create(&commander, NULL, commander_task, NULL) != 0)
        error_exit("creating commander thread failure");

    struct epoll_event event;

    while(1){
        if(epoll_wait(epoll, &event, 1, -1) == -1)
            error_exit("epoll wait failure");

        handle_message(event.data.fd);
    }
}

void handle_message(int socket){
    Message message;
    struct sockaddr* address = malloc(sizeof(struct sockaddr));
    socklen_t address_size = sizeof(struct sockaddr);


    if (recvfrom(socket, &message, sizeof(Message), 0, address, &address_size) != sizeof(Message))
        error_exit("reading message type failure");


    switch (message.type) {

        case LOGIN:
            login_handler(socket, address, address_size, message);
            break;


        case LOGOUT:
            pthread_mutex_lock(&client_mutex);

            for (int j = 0; j < client_number; ++j) {
                if (strcmp(clients_info[j].name, message.name) == 0){
//                    printf("client %s logged out\n", client_name);
                    remove_client(j);
                }
            }
            pthread_mutex_unlock(&client_mutex);
            break;


        case RESULT:
            {
            int calculation_result = ntohl((uint32_t) message.result);
            int task_counter = ntohl((uint32_t) message.counter);

             printf("Client \"%s\" send result of task %d: %d\n\n", message.name, task_counter, calculation_result);

             break;
            }


        case PONG:
            pthread_mutex_lock(&client_mutex);
            for (int i = 0; i < client_number; ++i) {
                if (strcmp(clients_info[i].name, message.name) == 0){
                    clients_info[i].ping_balance --;
                    clients_info[i].received_counter ++;
                    break;
                }
            }
            pthread_mutex_unlock(&client_mutex);
            break;

        default:
            printf("unknown message\n");
    }


}

void login_handler(int socket, struct sockaddr *address, socklen_t address_size, Message message){
    pthread_mutex_lock(&client_mutex);

    if (client_number == MAX_CLIENTS){
        char message_type = FAILSIZE;
        if (sendto(socket, &message_type, 1, 0, address, address_size) != 1)
            error_exit("write failsize message failure");
        free(address);
        pthread_mutex_unlock(&client_mutex);
        return;
    }

    for (int i = 0; i < client_number; ++i) {
        char message_type = FAILNAME;
        if (strcmp(clients_info[i].name, message.name) == 0){
            if (sendto(socket, &message_type, 1, 0, address, address_size) != 1)
                error_exit("write failname message failure");
            free(address);
            pthread_mutex_unlock(&client_mutex);
            return;
        }
    }

    clients_info[client_number].address = address;
    clients_info[client_number].name = malloc(strlen(message.name) * sizeof(char) + 1);
    strcpy(clients_info[client_number].name, message.name);
    clients_info[client_number].received_counter = 0;
    clients_info[client_number].ping_balance = 0;
    clients_info[client_number].connection_type = message.connection_type;
    clients_info[client_number].address_size = address_size;
    client_number ++;

    char message_type = SUCCESS;
    if (sendto(socket, &message_type, 1, 0, address, address_size) != 1)
        error_exit("write success login message failure");
//    printf("Client \"%s\" registered\n", clients_info[client_number - 1].name);

    pthread_mutex_unlock(&client_mutex);

}

void error_exit(char *error_message) {
    perror(error_message);
    exit(EXIT_FAILURE);
}

void* commander_task(void* args){
    int arg1, arg2;
    char operator;
    char buffer[256];

    while (1) {

        fprintf(stdout, "Enter arithmetic expression: ");
        fgets(buffer, 256, stdin);
        fprintf(stdout, "\n");

        if (sscanf(buffer, "%d %c %d", &arg1, &operator, &arg2) != 3){
            printf("Invalid arithmetic expression. Use: <arg1> <op> <arg2>\n");
            continue;
        }

        if (operator != '+' && operator != '-' && operator != '*' && operator != '/'){
            printf("Invalid operator. Use + - * /");
            continue;
        }

        pthread_mutex_lock(&client_mutex);

        Client_info random_client = clients_info[rand() % client_number];
        int tmp_counter = htonl((uint32_t) messages_send_conter++);

        int socket = random_client.connection_type == WEB ? web_socket : local_socket;
        char message_type = REQUEST;
        arg1 = htonl((uint32_t) arg1);
        arg2 = htonl((uint32_t) arg2);

        int success = 1;
        if (sendto(socket, &message_type, 1, 0, random_client.address, random_client.address_size) != 1) success = 0;
        if (sendto(socket, &tmp_counter, sizeof(int), 0, random_client.address, random_client.address_size) != sizeof(int)) success = 0;
        if (sendto(socket, &arg1, sizeof(int), 0, random_client.address, random_client.address_size) != sizeof(int)) success = 0;
        if (sendto(socket, &operator, 1, 0, random_client.address, random_client.address_size) != 1) success = 0;
        if (sendto(socket, &arg2, sizeof(int), 0, random_client.address, random_client.address_size) != sizeof(int)) success = 0;

        pthread_mutex_unlock(&client_mutex);

        if (success)
            usleep(1000);
//            printf("message send to client \"%s\"\n\n", random_client.name);
        else
            printf("message send failure\n\n");


    }
}

void* pinger_task(void* args){
    while (1) {
        pthread_mutex_lock(&client_mutex);
        for (int i = 0; i < client_number; ++i) {
            if (clients_info[i].ping_balance != 0){
                printf("Client \"%s\" has been disconnected\n", clients_info[i].name);
                printf("Unregistered client %s\n", clients_info[i].name);
                remove_client(i);
                i--;
            } else {
                char message_type = PING;
                int socket = clients_info[i].connection_type == WEB ? web_socket : local_socket;
                if (sendto(socket, &message_type, 1, 0, clients_info[i].address, clients_info[i].address_size) != 1)
                    printf("Error while sending PING to \"%s\"\n", clients_info[i].name);
                clients_info[i].ping_balance ++;
            }
        }
        pthread_mutex_unlock(&client_mutex);
        sleep(2);
    }
}

void validate_args(char **args) {
    int socket_number = atoi(args[1]);
    char *unix_path = args[2];

    if ((socket_number < 1024) || (socket_number > 60999)) {
        printf("Invalid socket number\n");
        exit(EXIT_FAILURE);
    }

    if (strlen(unix_path) == 0 || strlen(unix_path) > UNIX_PATH_MAX){
        printf("Invalid unix path\n");
        exit(EXIT_FAILURE);
    }
}

int initialise_web_socket(int socket_number){
    struct sockaddr_in web_address;
    web_address.sin_family = AF_INET;
    web_address.sin_port = htons((uint16_t) socket_number);
    web_address.sin_addr.s_addr = INADDR_ANY;

    int web_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (web_socket == -1)
        error_exit("web socket creation failure");

    if (bind(web_socket, (const struct sockaddr *) &web_address, sizeof(web_address)) == 1)
        error_exit("web socket bind failure");

    printf("Web socket initialised with address: %s\n", inet_ntoa(web_address.sin_addr));

    return web_socket;
}

int initialise_local_socket(char *unix_path){
    struct sockaddr_un local_address;
    local_address.sun_family = AF_UNIX;
    strcpy(local_address.sun_path, unix_path);

    int local_socket = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (local_socket == -1)
        error_exit("local socket creation failure");

    if (bind(local_socket, (const struct sockaddr *) &local_address, sizeof(local_address)))
        error_exit("local socket bind failure");

    printf("Local socket initialised\n");

    return local_socket;
}

int initialise_epoll_monitor(){
    int epoll = epoll_create1(0);
    if (epoll == -1)
        error_exit("epoll monitor creation failure");

    struct epoll_event epoll_event1;
    epoll_event1.events = EPOLLIN | EPOLLPRI;
    epoll_event1.data.fd = web_socket;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, web_socket, &epoll_event1) == -1)
        error_exit("adding web socket under epoll monitoring failure");

    epoll_event1.data.fd = local_socket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, local_socket, &epoll_event1) == -1)
        error_exit("adding local socket under epoll monitoring failure");

    return epoll;
}

void remove_client(int position){
    free(clients_info[position].address);
    free(clients_info[position].name);
    for (int i = position; i <= client_number; ++i) {
        clients_info[i] = clients_info[i+1];
    }
    client_number --;
}

void clean_exit(){

    if(close(web_socket) == -1){
        perror("closing web socket failure");
    }

    if(close(local_socket) == -1){
        perror("closing local socket failure");
    }

    if(unlink(unix_path) == -1){
        perror("unlink unix path file failure");
    }

    if(close(epoll) == -1){
        perror("closing epoll failure");
       }

       printf("Server closed\n");
}

void sigint_handler(int signo){

    exit(EXIT_SUCCESS);
}



