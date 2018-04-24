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


int sessionID = -2;
mqd_t serverQueueID = -1;
mqd_t clientQueueID = -1;
char path[32];
const char serverPath[] = "/queueServer";

typedef enum messageType{
    LOGIN = 1, MIRROR = 2, CALC = 3, TIME = 4, END = 5, INIT = 6
} messageType;

typedef struct Message{
    long mtype;
    pid_t clientPID;
    char content[CONTENT_SIZE];
} Message;

const size_t MSG_SIZE = sizeof(Message) - sizeof(long) ;

void errorExit(char *message) {
    printf("%s\n", message);
    perror("");
    exit(EXIT_FAILURE);
}

void logIntoServer(){
    Message message;
    message.mtype = LOGIN;
    message.clientPID = getpid();

    if(mq_send(serverQueueID, (char *) &message, MSG_SIZE, 1) == -1) {
        errorExit("could not send logging request to server");
    }
    if(mq_receive(clientQueueID, (char *) &message, MSG_SIZE, NULL) == -1) {
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
    if(mq_send(serverQueueID, (char *) message, MSG_SIZE, 1) == -1) {
        errorExit("could not send mirror request to server");
    }
    if(mq_receive(clientQueueID, (char *) message, MSG_SIZE, NULL) == -1) {
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
    if(mq_send(serverQueueID, (char *) message, MSG_SIZE, 0) == -1) {
        errorExit("could not send calc request to server");
    }
    if(mq_receive(clientQueueID, (char *) message, MSG_SIZE, NULL) == -1) {
        errorExit("could not receive calc response from server");
    }
    printf("%s", message->content);
}

void timeRequest(Message *message){
    message->mtype = TIME;

    if(mq_send(serverQueueID, (char *) message, MSG_SIZE, 1) == -1) {
        errorExit("could not send time request to server");
    }
    if(mq_receive(clientQueueID, (char *) message, MSG_SIZE, NULL) == -1) {
        errorExit("could not receive time response from server");
    }
    printf("%s\n", message->content);
}

void endRequest(Message *message){
    message->mtype = END;
    if(mq_send(serverQueueID, (char *) message, MSG_SIZE, 1) == -1) {
        errorExit("could not send end request");
    }
}

void removeQueue(){
    if(clientQueueID > -1){

        if(mq_close(clientQueueID) == -1){
            printf("could not close client queue\n");
        }
        else printf("client queue successfully closed\n");

        if(mq_close(serverQueueID) == -1){
            printf("could not close server queue\n");
        }

        if(mq_unlink(path) == -1){
            printf("could not delete client queue\n");
        }
        else printf("client queue successfully deleted\n");
    } else printf("client queue already not exists\n");
}

void intAction(int signo){
    printf("received signal %d\n", signo);
    exit(2);
}

void handleSession(){
    char request[32];
    Message message;
    while(1){
        message.clientPID = getpid();
        printf("Enter request: ");
        if(fgets(request, 32, stdin) == NULL){
            printf("could not read request\n");
            continue;
        }
        int n = (int) strlen(request);
        if(request[n-1] == '\n') request[n-1] = 0;


        if(strcmp(request, "mirror") == 0){
            mirrorRequest(&message);
        }else if(strcmp(request, "calc") == 0){
            calcRequest(&message);
        }else if(strcmp(request, "time") == 0){
            timeRequest(&message);
        }else if(strcmp(request, "end") == 0){
            endRequest(&message);
            exit(EXIT_SUCCESS);
        }else printf("Wrong command!\n");
    }
}

int main(){

    if(atexit(removeQueue) == -1) {
        errorExit("could not set 'atexit' queue remove function");
    }
    if(signal(SIGINT, intAction) == SIG_ERR) {
        errorExit("could not set signal function");
    }

    sprintf(path, "/%d", getpid());

    serverQueueID = mq_open(serverPath, O_WRONLY);
    if(serverQueueID == -1) errorExit("could not open server queue\n");

    struct mq_attr posixAttr;
    posixAttr.mq_maxmsg = MAX_MQSIZE;
    posixAttr.mq_msgsize = MSG_SIZE;

    clientQueueID = mq_open(path, O_RDONLY | O_CREAT | O_EXCL, 0666, &posixAttr);
    if(clientQueueID == -1) errorExit("could not create client queue");

    logIntoServer();

    handleSession();

}




