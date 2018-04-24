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
#define SERVER_ID 244
#define CONTENT_SIZE 4096

int serverQueueID = -99;
int inUse = 1;
int clientsInfo[MAX_CLIENTS][2];
int clientsNumber = 0;

typedef enum messageType{
    LOGIN = 1, MIRROR = 2, CALC = 3, TIME = 4, END = 5, INIT = 6
} messageType;

typedef struct Message{
    long mtype;
    pid_t clientPID;
    char content[CONTENT_SIZE];
} Message;

const size_t MSG_SIZE = sizeof(Message) - sizeof(long);

void errorExit(char *message) {
    printf("%s\n", message);
    perror("");
    exit(EXIT_FAILURE);
}

int findQueueID(pid_t serverPID){
    for(int i=0; i<MAX_CLIENTS; i++){
        if(clientsInfo[i][0] == serverPID)
            return clientsInfo[i][1];
    }
    return -1;
}

char* convertTime(const time_t* time){
    char* buff = malloc(sizeof(char) * 30);
    struct tm *localTime;
    localTime = localtime (time);
    strftime(buff, 20, "%b %d %H:%M", localTime);
    return buff;
}

int buildMessage(Message *message){
    int clientQID = findQueueID(message->clientPID);
    if(clientQID == -1){
        printf("could not find client\n");
        return -1;
    }

    message->mtype = message->clientPID;
    message->clientPID = getpid();

    return clientQID;
}

void loginService(Message *message){
    key_t clientQueueKey;
    if(sscanf(message->content, "%d", &clientQueueKey) < 0)
        errorExit("could not receive client queue key");

    int clientQueueID = msgget(clientQueueKey, 0);
    if(clientQueueID == -1 )
        errorExit("could not receive client queue id");

    int clientPID = message->clientPID;
    message->mtype = INIT;
    message->clientPID = getpid();

    if(clientsNumber > MAX_CLIENTS - 1){
        printf("too many clients are already logged in\n");
        sprintf(message->content, "%d", -1);
    }else{
        clientsInfo[clientsNumber][0] = clientPID;
        clientsInfo[clientsNumber++][1] = clientQueueID;
        sprintf(message->content, "%d", clientsNumber-1);
    }

    if(msgsnd(clientQueueID, message, MSG_SIZE, 0) == -1)
        errorExit("Error during logging in");
}

void mirrorService(Message *message){
    int clientQueueID = buildMessage(message);
    if(clientQueueID == -1) return;

    int msgLen = (int) strlen(message->content);
    if(message->content[msgLen-1] == '\n') msgLen--;

    for(int i=0; i < msgLen / 2; i++) {
        char buff = message->content[i];
        message->content[i] = message->content[msgLen - i - 1];
        message->content[msgLen - i - 1] = buff;
    }

    if(msgsnd(clientQueueID, message, MSG_SIZE, 0) == -1) errorExit("could not send mirror response");
}

void calcService(Message *message){
    int clientQueueID = buildMessage(message);
    if(clientQueueID == -1) return;

    char cmd[4108];
    sprintf(cmd, "echo '%s' | bc", message->content);
    FILE* calc = popen(cmd, "r");
    fgets(message->content, CONTENT_SIZE, calc);
    pclose(calc);

    if(msgsnd(clientQueueID, message, MSG_SIZE, 0) == -1) errorExit("could not send calc response");
}

void timeService(Message *message){
    int clientQID = buildMessage(message);
    if(clientQID == -1) return;

    time_t timer;
    time(&timer);
    char* timeStr = convertTime(&timer);

    sprintf(message->content, "%s", timeStr);
    free(timeStr);

    if(msgsnd(clientQID, message, MSG_SIZE, 0) == -1) errorExit("could not send time response");
}

void serverExecuteService(Message *message){
    if(message == NULL) return;
    switch(message->mtype){
        case LOGIN:
            loginService(message);
            break;
        case MIRROR:
            mirrorService(message);
            break;
        case CALC:
            calcService(message);
            break;
        case TIME:
            timeService(message);
            break;
        case END:
            inUse = 0;
            break;
        default:
            break;
    }
}

void removeQueue(){
    if(serverQueueID > -1){
        int tmp = msgctl(serverQueueID, IPC_RMID, NULL);
        if(tmp == -1){
            printf("could not remove server queue\n");
        }
        printf("server queue removed\n");
    }
}

void intAction(int signo){
    printf("received signal %d\n", signo);
    exit(SIGINT);
}

int main(){
    if(atexit(removeQueue) == -1)
        errorExit("could not set 'atexit' queue removal function");
    if(signal(SIGINT, intAction) == SIG_ERR)
        errorExit("could not set signal function");

    Message message;
    struct msqid_ds queueState;

    char* path = getenv("HOME");
    if(path == NULL) {
        errorExit("could not get HOME variable");
    }

    key_t serverKey = ftok(path, SERVER_ID);
    if(serverKey == -1) {
        errorExit("could not get server key from ftok");
    }

    serverQueueID = msgget(serverKey, IPC_CREAT | IPC_EXCL | 0666);
    if(serverQueueID == -1) {
        errorExit("could not create server queue");
    }

    while(1){
        if(inUse == 0){
            if(msgctl(serverQueueID, IPC_STAT, &queueState) == -1)
                errorExit("could not get state of server queue\n");
            if(queueState.msg_qnum == 0) break;
        }

        if(msgrcv(serverQueueID, &message, MSG_SIZE, 0, 0) < 0) {
            errorExit("could not receive message");
        }
        serverExecuteService(&message);
    }

    return 0;
}







