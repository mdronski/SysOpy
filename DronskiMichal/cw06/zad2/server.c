#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <mqueue.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/msg.h>

#define MAX_CLIENTS  16
#define CONTENT_SIZE 4096
#define MAX_MQSIZE 9

int serverQueueID = -2;
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

const size_t MSG_SIZE = sizeof(Message) - sizeof(long) ;
const char serverPath[] = "/queueServer";

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
    int clientQueueID = findQueueID(message->clientPID);
    if(clientQueueID == -1){
        printf("could not find a client\n");
        return -1;
    }

    message->mtype = message->clientPID;
    message->clientPID = getpid();

    return clientQueueID;
}

void loginService(Message *message){
    int clientPID = message->clientPID;
    char clientPath[15];
    sprintf(clientPath, "/%d", clientPID);

    int clientMQ_Desc = mq_open(clientPath, O_WRONLY);
    if(clientMQ_Desc == -1 ) {
        errorExit("could not open client queue");
    }

    message->mtype = INIT;
    message->clientPID = getpid();

    if(clientsNumber > MAX_CLIENTS - 1){
        printf("Maximum amount of clients reached!\n");
        sprintf(message->content, "%d", -1);
        if(mq_send(clientMQ_Desc, (char*) message, MSG_SIZE, 1) == -1) {
            errorExit("could not send login response");
        }
        if(mq_close(clientMQ_Desc) == -1) {
            errorExit("could not close client queue");
        }
    }else{
        clientsInfo[clientsNumber][0] = clientPID;
        clientsInfo[clientsNumber++][1] = clientMQ_Desc;
        sprintf(message->content, "%d", clientsNumber-1);
        if(mq_send(clientMQ_Desc, (char*) message, MSG_SIZE, 1) == -1) {
            errorExit("could not send login response");
        }
    }

}

void mirrorService(Message *message){
    int clientQueueID = buildMessage(message);
    if(clientQueueID == -1) return;

    int messageLength = (int) strlen(message->content);
    if(message->content[messageLength-1] == '\n') messageLength--;

    for(int i=0; i < messageLength / 2; i++) {
        char buff = message->content[i];
        message->content[i] = message->content[messageLength - i - 1];
        message->content[messageLength - i - 1] = buff;
    }

    if(mq_send(clientQueueID, (char*) message, MSG_SIZE, 1) == -1) {
        errorExit("could not send mirror response");
    }
}

void calcService(Message *message){
    int clientQueueID = buildMessage(message);
    if(clientQueueID == -1) return;

    char cmd[4090];
    sprintf(cmd, "echo '%s' | bc", message->content);
    FILE* bashCalculator = popen(cmd, "r");
    fgets(message->content, CONTENT_SIZE, bashCalculator);
    pclose(bashCalculator);

    if(mq_send(clientQueueID, (char*) message, MSG_SIZE, 1) == -1) {
        errorExit("could not send calc response");
    }

}

void timeService(Message *message){
    int clientQueueID = buildMessage(message);
    if(clientQueueID == -1) return;

    time_t timeNow;
    time(&timeNow);
    char* formattedTime = convertTime(&timeNow);

    sprintf(message->content, "%s", formattedTime);
    free(formattedTime);

    if(mq_send(clientQueueID, (char *) message, MSG_SIZE, 1) == -1) {
        errorExit("could not send time response");
    }

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
    for(int i=0; i<clientsNumber; i++){
        if(mq_close(clientsInfo[i][1]) == -1){
            printf("could not close client(%d) queue\n", i);
        }
        kill(clientsInfo[i][0], SIGINT);

    }
    if(serverQueueID > -1){
        if(mq_close(serverQueueID) == -1){
            printf("could not close server queue\n");
        }
        else printf("server queue successfully closed\n");

        if(mq_unlink(serverPath) == -1) printf("could not delete server queue\n");
        else printf("server queue successfully deleted \n");
    }else printf("server queue already not exists\n");
}

void intAction(int signo){
    printf("received signal %d\n", signo);
    exit(SIGINT);
}

int main(){
    if(atexit(removeQueue) == -1)
        errorExit("could not set 'atexit' queue remove function");
    if(signal(SIGINT, intAction) == SIG_ERR)
        errorExit("could not set signal function");

    struct mq_attr currentState;

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MQSIZE;
    attr.mq_msgsize = MSG_SIZE;

    serverQueueID = mq_open(serverPath, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if(serverQueueID == -1) errorExit("could not create server queue");

    Message message;
    while(1){
        if(inUse == 0){
            if(mq_getattr(serverQueueID, &currentState) == -1) errorExit("could not read public queue parameters");
            if(currentState.mq_curmsgs == 0) exit(0);
        }

        if(mq_receive(serverQueueID,(char*) &message, MSG_SIZE, NULL) == -1) errorExit("server could not receive a message");
        serverExecuteService(&message);
    }
}







