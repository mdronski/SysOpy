#include <stdio.h>
#include <bits/types/time_t.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>

pid_t childProc;

void stpAction(int sigNum){

    if (kill(childProc, 0) == 0){
        if(kill(childProc, 9) != 0){
            perror("Unable to send SIGKILL to child process");
        }else {
            wait(NULL);
            printf("\nOczekuję na:\nCTRL+Z - kontynuacja\nCTRL+C - zakończenie programu\n");
        }
    } else {
        childProc = fork();
        if (childProc == -1){
            perror("Fork error");
            exit(EXIT_FAILURE);
        }
        if (childProc == 0) {
            execlp("./date.zsh", NULL);
        }
    }
}

void intAction(int sigNum){
    printf("\nOdebrano sygnał SIGINT\n");
    if (childProc == 0) kill(childProc, SIGINT);
    exit(EXIT_SUCCESS);
}

int main() {
    int exitStatus;
    struct sigaction newAction;
    newAction.sa_handler = stpAction;
    newAction.sa_flags = 0;
    sigfillset(&newAction.sa_mask);
    sigdelset(&newAction.sa_mask, SIGTSTP);
    sigdelset(&newAction.sa_mask, SIGINT);

    sigaction(SIGTSTP, &newAction, NULL);
    signal(SIGINT, &intAction);

    childProc = fork();
    if (childProc == 0) {
        execlp("./date.zsh", "date.zsh", NULL);
    }
    while(1) pause();

    return 0;
}