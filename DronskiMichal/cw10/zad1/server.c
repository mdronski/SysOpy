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

typedef struct Client_info {
    int socket_desc;
    char *name;
    int ping_balance;
    int received_counter;
}Client_info ;

typedef enum Message_Type{
    LOGIN = 0,
    LOGOUT = 1,
    REQUEST = 2,
    RESULT = 3,
    SUCCESS = 4,
    FAILSIZE = 5,
    FAILNAME = 6,
    PING = 7,
    PONG = 8,
}Message_Type;

typedef enum Connection_Type{
    LOCAL = 0,
    WEB = 1
}Connection_Type;


int web_socket;
int local_socket;
int epoll;
int client_number = 0;
int messages_send_conter = 0;

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
void remove_socket(int socket);
void register_client(int socket);
void handle_message(int socket);
void login_handler(int socket, char *client_name);

int main(int argc, char** argv) {
    atexit(clean_exit);
    srand((unsigned int) time(NULL));

    validate_args(argv);
    int socket = atoi(argv[1]);
    char *unix_path = argv[2];

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

        if(event.data.fd < 0){
            register_client(-event.data.fd);
        }else{
            handle_message(event.data.fd);
        }
    }
}

void handle_message(int socket){
    enum Message_Type message_type;
    int client_name_length;
    char *client_name;


    if (read(socket, &message_type, sizeof(message_type)) != sizeof(message_type))
        error_exit("reading message type failure");

    if (read(socket, &client_name_length, sizeof(client_name_length)) != sizeof(client_name_length));
        error_exit("reading message length failure");

    client_name_length = ntohs((uint16_t) client_name_length);
    client_name = malloc(client_name_length * sizeof(char));

    switch (message_type) {

        case LOGIN:
            if(read(socket, client_name,  client_name_length) != client_name_length)
                error_exit("reading new client name in login handler failure");
            login_handler(socket, client_name);
            break;


        case LOGOUT:
            pthread_mutex_lock(&client_mutex);

            if(read(socket, client_name,  client_name_length) != client_name_length)
                error_exit("reading logging out client name failure");

            for (int j = 0; j < client_number; ++j) {
                if (strcmp(clients_info[j].name, client_name) == 0){
                    printf("client %s logged out\n", client_name);
                    remove_client(j);
                }
            }
            break;


        case RESULT:
            {
            int calculation_result, task_counter;

            if (read(socket, &task_counter, sizeof(task_counter)) != sizeof(task_counter))
                error_exit("reading calculation counter failure");
            task_counter = ntohl(task_counter);

            if (read(socket, &calculation_result, sizeof(calculation_result)) != sizeof(calculation_result))
                error_exit("reading calculation counter failure");
            calculation_result = ntohl(calculation_result);

            if(read(socket, client_name,  client_name_length) != client_name_length)
                error_exit("reading new client name in result handler failure");

            printf("Client %s send result of task %d: %d\n\n", client_name, task_counter, calculation_result);
            break;
            }


        case PONG:
            pthread_mutex_lock(&client_mutex);
            if(read(socket, client_name,  client_name_length) != client_name_length)
                error_exit("reading new client name in pong handler failure");

            for (int i = 0; i < client_number; ++i) {
                if (strcmp(clients_info[i].name, client_name) == 0){
                    clients_info[i].ping_balance --;
                    clients_info[i].received_counter ++;
                    break;
                }
            }
            break;

    }

    free(client_name);

}

void login_handler(int socket, char *client_name){
    pthread_mutex_lock(&client_mutex);

    if (client_number == MAX_CLIENTS){
        if (write(socket, &FAILSIZE, sizeof(FAILSIZE)) != sizeof(FAILSIZE))
            error_exit("write failsize message failure");
        remove_socket(socket);
        pthread_mutex_unlock(&client_mutex);
        return;
    }

    for (int i = 0; i < client_number; ++i) {
        if (strcmp(clients_info[i].name, client_name) == 0){
            if (write(socket, &FAILNAME, sizeof(FAILNAME)) != sizeof(FAILNAME))
                error_exit("write failname message failure");
            remove_socket(socket);
            pthread_mutex_unlock(&client_mutex);
            return;
        }
    }

    clients_info[client_number].socket_desc = socket;
    clients_info[client_number].name = malloc(strlen(client_name) * sizeof(char) + 1);
    strcpy(clients_info[client_number].name, client_name);
    clients_info[client_number].received_counter = 0;
    clients_info[client_number].ping_balance = 0;

    client_number ++;

    if (write(socket, &SUCCESS, sizeof(SUCCESS)) != sizeof(SUCCESS))
        error_exit("write success login message failure");

    pthread_mutex_unlock(&client_mutex);

}

void error_exit(char *error_message) {
    perror(error_message);
    exit(EXIT_FAILURE);
}

void register_client(int socket){
    int new_client = accept(socket, NULL, NULL);
    if (new_client == -1)
        error_exit("client registration failure");

    struct epoll_event epoll_event1;
    epoll_event1.events = EPOLLIN | EPOLLPRI;
    epoll_event1.data.fd = new_client;

    if(epoll_ctl(epoll, EPOLL_CTL_ADD, new_client, &epoll_event1) == -1)
        error_exit("adding new client under epoll monitoring failure");
}

void* commander_task(void* args){
    int arg1, arg2;
    char operator;
    char buffer[256];

    while (1) {
        printf("Enter arithmetic expression: ");
        fgets(buffer, 256, stdin);

        if (sscanf(buffer, "%d %c %d", &arg1, &operator, &arg2) != 3){
            printf("Invalid arithmetic expression. Use: <argument1> <operator> <argument2>");
            continue;
        }

        if (operator != '+' || operator != '-' || operator != '*' || operator != '/'){
            printf("Invalid operator. Use + - * /");
            continue;
        }

        pthread_mutex_lock(&client_mutex);

        Client_info random_client = clients_info[rand() % client_number];
        int tmp_counter = htonl((uint32_t) messages_send_conter++);
        int converted_arg1 = htonl((uint32_t) arg1);
        int converted_arg2 = htonl((uint32_t) arg2);

        int success = 1;
        if (write(random_client.socket_desc, &REQUEST, sizeof(REQUEST)) != sizeof(REQUEST)) success = 0;
        if (write(random_client.socket_desc, &tmp_counter, sizeof(tmp_counter)) != sizeof(tmp_counter)) success = 0;
        if (write(random_client.socket_desc, &converted_arg1, sizeof(converted_arg1)) != sizeof(converted_arg1)) success = 0;
        if (write(random_client.socket_desc, &operator, sizeof(operator)) != sizeof(operator)) success = 0;
        if (write(random_client.socket_desc, &converted_arg2, sizeof(converted_arg2)) != sizeof(converted_arg2)) success = 0;

        pthread_mutex_unlock(&client_mutex);

        if (success)
            printf("message send to client %s\n\n", random_client.name);
        else
            printf("message send failure\n\n");
    }
}

void* pinger_task(void* args){
    while (1) {
        pthread_mutex_lock(&client_mutex);
        for (int i = 0; i < client_number; ++i) {
            if (clients_info[i].ping_balance != 0){
                printf("Client \"%s\" has been disconnected", clients_info[i].name);
                remove_client(i);
                i--;
            } else {
                if (write(clients_info[i].socket_desc, &PING, sizeof(PING)) != sizeof(PING))
                    printf("Error while sending PING to \"%s\"\n", clients_info[i].name);
                clients_info[i].ping_balance ++;
            }
        }
        pthread_mutex_unlock(&client_mutex);
        sleep(5);
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

    int web_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (web_socket == -1)
        error_exit("web socket creation failure");

    if (bind(web_socket, (const struct sockaddr *) &web_address, sizeof(web_address)) == 1)
        error_exit("web socket bind failure");

    if (listen(web_socket, MAX_CLIENTS) == -1)
        error_exit("web socket listen failure");

    printf("Server initialised with address: %s\n", inet_ntoa(web_address.sin_addr));

    return web_socket;
}

int initialise_local_socket(char *unix_path){
    struct sockaddr_un local_address;
    local_address.sun_family = AF_UNIX;
    strcpy(local_address.sun_path, unix_path);

    int local_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (local_socket == -1)
        error_exit("local socket creation failure");

    if (bind(local_socket, (const struct sockaddr *) &local_address, sizeof(local_address)))
        error_exit("local socket bind failure");

    if (listen(local_socket, MAX_CLIENTS) == -1)
        error_exit("local socket listen failure");

    printf("Local socket initialised\n");

    return local_socket;
}

int initialise_epoll_monitor(){
    int epoll = epoll_create1(0);
    if (epoll == -1)
        error_exit("epoll monitor creation failure");

    struct epoll_event epoll_event1;
    epoll_event1.events = EPOLLIN | EPOLLPRI;
    epoll_event1.data.fd = -web_socket;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, web_socket, &epoll_event1) == -1)
        error_exit("adding web socket under epoll monitoring failure");

    epoll_event1.data.fd = -local_socket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, local_socket, &epoll_event1) == -1)
        error_exit("adding local socket under epoll monitoring failure");

    return epoll;
}

void remove_client(int position){
    remove_socket(clients_info[position].socket_desc);
    free(clients_info[position].name);
    for (int i = position; i <= client_number; ++i) {
        clients_info[i] = clients_info[i+1];
    }
    client_number --;
}

void remove_socket(int socket){
    if(epoll_ctl(epoll, EPOLL_CTL_DEL, socket, NULL) == -1)
        error_exit("removing socket from epoll monitor failure");
    if(shutdown(socket, SHUT_RDWR) == -1)
        error_exit("socket shutdown failure");
    if(close(socket) == -1)
        error_exit("socket close failure");
}

void clean_exit(){

    if(close(web_socket) == -1){
        perror("closing web socket failure");
    }

    if(close(local_socket) == -1){
        perror("closing local socket failure");
    }

    if(close(epoll) == -1){
        perror("closing epoll failure");
       }
}