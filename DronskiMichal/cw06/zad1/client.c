#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#define MAX_CLIENTS  16
#define SERVER_ID 31415
#define CONTENT_SIZE 4096

int sessionID = -2;
int serverQueueID = -1;
int clientQueueID = -1;

typedef enum messageType{
    LOGIN = 1, MIRROR = 2, CALC = 3, TIME = 4, END = 5, INIT = 6
} messageType;

typedef struct Message{
    long mtype;
    pid_t clientPID;
    char content[CONTENT_SIZE];
} Message;

const size_t MSG_SIZE = sizeof(Message) ;

void errorExit(char *message) {
    printf("%s\n", message);
    perror("");
    exit(EXIT_FAILURE);
}

void logIntoServer(key_t clientKey){
    Message message;
    message.mtype = LOGIN;
    message.clientPID = getpid();
    sprintf(message.content, "%d", clientKey);

    if(msgsnd(serverQueueID, &message, MSG_SIZE, 0) == -1) {
        errorExit("could not send logging request to server");
    }
    if(msgrcv(clientQueueID, &message, MSG_SIZE, 0, 0) == -1) {
        errorExit("couldn't receive logging response");
    }
    if(sscanf(message.content, "%d", &sessionID) < 1) {
        errorExit("could not parse server response");
    }
    if(sessionID < 0) {
        errorExit("there is already too much clients logged in");
    }

    printf("successfully logged into the server, session number: %d\n", sessionID);
}

void mirrorRequest(Message *message){
    message->mtype = MIRROR;
    printf("enter string to be mirrored: ");
    if(fgets(message->content, CONTENT_SIZE, stdin) == NULL){
        printf("this string is too long\n");
        return;
    }
    if(msgsnd(serverQueueID, message, MSG_SIZE, 0) == -1) {
        errorExit("could not send mirror request to server");
    }
    if(msgrcv(clientQueueID, message, MSG_SIZE, 0, 0) == -1) {
        errorExit("could not receive mirror response from server");
    }
    printf("%s", message->content);
}

void calcRequest(Message *message){
    message->mtype = CALC;
    printf("Enter expression to calculate: ");
    if(fgets(message->content, CONTENT_SIZE, stdin) == NULL){
        printf("it is too long!\n");
        return;
    }
    if(msgsnd(serverQueueID, message, MSG_SIZE, 0) == -1) {
        errorExit("could not send calc request to server");
    }
    if(msgrcv(clientQueueID, message, MSG_SIZE, 0, 0) == -1) {
        errorExit("could not receive calc response from server");
    }
    printf("%s", message->content);
}

void timeRequest(Message *message){
    message->mtype = TIME;

    if(msgsnd(serverQueueID, message, MSG_SIZE, 0) == -1) {
        errorExit("could not send time request to server");
    }
    if(msgrcv(clientQueueID, message, MSG_SIZE, 0, 0) == -1) {
        errorExit("could not receive time response from server");
    }
    printf("%s\n", message->content);
}

void endRequest(Message *message){
    message->mtype = END;
    if(msgsnd(serverQueueID, message, MSG_SIZE, 0) == -1) {
        errorExit("could not send end request");
    }
}

int openServerQueue(char *path, int ID){
    int key = ftok(path, ID);
    if(key == -1) errorExit("Error during generating queue key");

    int QueueID = msgget(key, 0);
    if(QueueID == -1) errorExit("Error during opening queue");

    return QueueID;
}

void removeQueue(void){
    if(clientQueueID > -1){
        if(msgctl(clientQueueID, IPC_RMID, NULL) == -1){
            printf("Error during removal of server queue\n");
        }
        else printf("\nclient queue removed\n");
    }
}

void intAction(int signo){
    printf("received signal %d\n", signo);
    exit(2);
}

void handleSession(){
    char service[32];
    Message message;

    while(1){
        message.clientPID = getpid();
        printf("Enter request: ");
        if(fgets(service, 32, stdin) == NULL){
            printf("Error reading your command!\n");
            continue;
        }
        int n = (int) strlen(service);
        if(service[n-1] == '\n') service[n-1] = 0;

        if(strcmp(service, "mirror") == 0){
            mirrorRequest(&message);
        }else if(strcmp(service, "calc") == 0){
            calcRequest(&message);
        }else if(strcmp(service, "time") == 0){
            timeRequest(&message);
        }else if(strcmp(service, "end") == 0){
            endRequest(&message);
            exit(EXIT_SUCCESS);
        }else printf("Wrong command!\n");
    }
}

int main(){

    if(atexit(removeQueue) == -1) {
        errorExit("Error during atexit function");
    }
    if(signal(SIGINT, intAction) == SIG_ERR) {
        errorExit("Error during signal");
    }

    char* path = getenv("HOME");
    if(path == NULL) {
        errorExit("Error during loading HOME to program");
    }

    serverQueueID = openServerQueue(path, SERVER_ID);

    key_t privateKey = ftok(path, getpid());
    if(privateKey == -1) {
        errorExit("Error during generation key for server queue");
    }

    clientQueueID = msgget(privateKey, IPC_CREAT | IPC_EXCL | 0666);
    if(clientQueueID == -1) {
        errorExit("Error during creating client private queue");
    }

    logIntoServer(privateKey);

    handleSession();
}




