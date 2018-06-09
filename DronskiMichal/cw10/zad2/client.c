#define UNIX_PATH_MAX 108

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

enum Connection_Type connection_type;
char *name;
char *unix_path;
int ip_address;
int port_number;
int socket_desc;

void clean_exit();
void error_exit(char *error_message);
void validate_and_initialise(int argc, char **argv);
void connect_to_server();
void login_to_server();
void send_message(enum Message_Type message_type, int counter, int  result);
void sigint_handler(int signo);
void handle_request();


int main(int argc, char** argv){
    atexit(clean_exit);
    signal(SIGINT, sigint_handler);
    validate_and_initialise(argc, argv);

    connect_to_server();

    login_to_server();

    while(1){
        char message_type;
        if(read(socket_desc, &message_type, 1) != 1)
            error_exit("reading server response message type failure");

        if(message_type == PING){
            send_message(PONG, 0, 0);
        }else if(message_type == REQUEST){
            handle_request();
        }
    }
}

void handle_request(){
    char operator;
    int arg1, arg2, result;
    int counter;

    if(read(socket_desc, &counter, sizeof(int)) != sizeof(int))
        error_exit("counter read failure");
    if(read(socket_desc, &arg1, sizeof(int)) != sizeof(int))
        error_exit("argument1 read failure");
    if(read(socket_desc, &operator, 1) != 1)
        error_exit("operator read failure");
    if(read(socket_desc, &arg2, sizeof(int)) != sizeof(int))
        error_exit("counter read failure");

    arg1 = ntohl((uint32_t) arg1);
    arg2 = ntohl((uint32_t) arg2);

    result = operator == '+' ? arg1 + arg2 :
             operator == '-' ? arg1 - arg2 :
             operator == '*' ? arg1 * arg2 :
             arg1 / arg2;

    printf("Task %d: %d %c %d = %d\n", ntohl((uint32_t) counter), arg1, operator, arg2, result);

    result = htonl((uint32_t) result);
    send_message(RESULT, counter, result);

}

void login_to_server(){
    char response_type;

    send_message(LOGIN, 0, 0);

    if(read(socket_desc, &response_type, 1) != 1)
        error_exit("login response failure");

    switch (response_type) {
        case FAILSIZE:
            printf("there is already too many clients registered in server\n");
            exit(EXIT_SUCCESS);
        case FAILNAME:
            printf("there is already client with that name\n");
            exit(EXIT_SUCCESS);

        case SUCCESS:
            printf("Successfully logged to server\n");
            break;
        default:
            printf("during login received invalid message from server\n");
    }
}

void send_message(enum Message_Type message_type, int counter, int result){

    Message message;

    for (int i = 0; i < 64; ++i) {
        message.name[i] = 0;
    }
    strcpy(message.name, name);

    message.counter = counter;
    message.result = result;
    message.connection_type = connection_type;
    message.type = message_type;

    if (write(socket_desc, &message, sizeof(Message)) != sizeof(Message))
        perror("message send failure");

}

void connect_to_server(){

    switch (connection_type) {
        case LOCAL: {
            struct sockaddr_un local_address;
            local_address.sun_family = AF_UNIX;
            strcpy(local_address.sun_path, unix_path);

            socket_desc = socket(AF_UNIX, SOCK_DGRAM, 0);
            if (socket_desc == -1)
                error_exit("local socket creation failure");

            if (bind(socket_desc,(const struct sockaddr *) &local_address, sizeof(sa_family_t)) == -1)
                error_exit("local bind failure");

            if (connect(socket_desc,(const struct sockaddr *) &local_address, sizeof(local_address)) == -1)
                error_exit("local connection failure");
            break;
        }

        case WEB: {
            struct sockaddr_in web_address;
            web_address.sin_family = AF_INET;
            web_address.sin_port = 0;
            web_address.sin_addr.s_addr = INADDR_ANY;

            socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
            if(socket_desc == -1)
                error_exit("web socket creation failure");

            if (bind(socket_desc, (const struct sockaddr *) &web_address, sizeof(web_address)) == -1)
                error_exit("web socket bind failure");

            web_address.sin_family = AF_INET;
            web_address.sin_port = htons((uint16_t) port_number);
            web_address.sin_addr.s_addr = htonl((uint32_t) ip_address);

            if(connect(socket_desc,(const struct sockaddr *) &web_address, sizeof(web_address)) == -1)
                error_exit("web connection failure");
            break;
        }
    }

    printf("Connected\n");
}

void validate_and_initialise(int argc, char **argv){
    if (argc != 4 && argc != 5)
        error_exit("invalid arguments");

    name = argv[1];

        char *connection = argv[2];

    if (strcmp(connection, "web") == 0){
        connection_type = WEB;
    } else if (strcmp(connection, "local") == 0){
        connection_type = LOCAL;
    } else
        error_exit("Invalid connection type");

    switch (connection_type){
        case LOCAL:{
            if (argc != 4)
                error_exit("invalid arguments");

            unix_path = argv[3];
            if (strlen(unix_path) == 0 || strlen(unix_path) > UNIX_PATH_MAX)
                error_exit("Invalid unix path");
        }
            break;

        case WEB:
            if (argc != 5)
                error_exit("Invalid arguments");

            ip_address = inet_addr(argv[3]);
            if (ip_address == -1)
                error_exit("Invalid ip address");

            port_number = atoi(argv[4]);
            if (port_number < 1024 || port_number > 60999)
                error_exit("Invalid port number");
            break;

    }
}

void error_exit(char *error_message) {
    perror(error_message);
    exit(EXIT_FAILURE);
}

void clean_exit(){


    if(close(socket_desc) == -1){
        perror("closing socket failure");
    }

    printf("\nExited\n");
}

void sigint_handler(int signo){

    send_message(LOGOUT, 0, 0);
    exit(EXIT_SUCCESS);

}
