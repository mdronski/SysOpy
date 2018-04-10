#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include <zconf.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <memory.h>


int N = 1000;
int K = 700;
int DEBUG = 0;
pid_t childProc;
int requestCounter = 0;
int startPermission = 0;
int sleepSec;

void DBGForkInfo(int pid) {
    if (DEBUG) {
        char buffer[512];
        sprintf(buffer, "Froked new process with PID: %d\n", childProc);
        write(1, buffer, strlen(buffer));
    }
}

static void usr1Action(int sigNum, siginfo_t* info, void* vp){
    if (childProc == 0){
        startPermission = 1;
    }else if (requestCounter < K){
            requestCounter++;
        if (DEBUG) {
            char buffer[512];
            sprintf(buffer, "Received start request from %d, request counter: %d\n", info->si_pid, requestCounter);
            write(1, buffer, strlen(buffer));
        }
        } else {
        if (DEBUG) {
            char buffer[512];
            sprintf(buffer, "Received start request from %d, allowed to start\n", info->si_pid);
            write(1, buffer, strlen(buffer));
        }
            kill(info->si_pid, SIGUSR1);
        }
}

void RTAction(int sigNum, siginfo_t* info, void* vp){
    if (DEBUG) {
        char buffer[512];
        sprintf(buffer, "\nReceived signal: %d from process: %d\n", sigNum, info->si_pid);
        write(1, buffer, strlen(buffer));
    }
}

void chldAction(int signum, siginfo_t *info, void *vp) {
    char buffer[512];
    if (info->si_code == CLD_EXITED) {
            sprintf(buffer, "\nChild proces with pid: %d exited with status: %d\n", info->si_pid, info->si_status);
            write(1, buffer, strlen(buffer));
    } else {
        sprintf(buffer, "\nChild proces with pid: %d terminated by signal: %d\n", info->si_pid, info->si_status);
        write(1, buffer, strlen(buffer));
    }
}

void intAction(int signum, siginfo_t *info, void *vp) {
    char buffer[512];
    sprintf(buffer, "\nReceived SIGINT\n");
    write(1, buffer, strlen(buffer));
    killpg(0, SIGINT);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

    char buffer[512];

    if (argc<3){
        printf("Wrong number of arguments!\n");
        exit(EXIT_FAILURE);
    }

    N = (int) strtol(argv[1], NULL, 10);
    K = (int) strtol(argv[2], NULL, 10);
    DEBUG = (int) strtol(argv[3], NULL, 10);


    struct sigaction sigAction;
    sigfillset(&sigAction.sa_mask);
    sigdelset(&sigAction.sa_mask, SIGUSR1);
    sigdelset(&sigAction.sa_mask, SIGINT);
    sigdelset(&sigAction.sa_mask, SIGCHLD);
    sigAction.sa_flags = SA_SIGINFO;

    sigAction.sa_sigaction = &usr1Action;
    sigaction(SIGUSR1, &sigAction, NULL);

    sigAction.sa_sigaction = &chldAction;
    sigaction(SIGCHLD, &sigAction, NULL);

    sigAction.sa_sigaction = &intAction;
    sigaction(SIGINT, &sigAction, NULL);

    sigAction.sa_sigaction = &RTAction;
    sigfillset(&sigAction.sa_mask);
    for (int i = SIGRTMIN;  i<= SIGRTMAX ; i++) {
        sigdelset(&sigAction.sa_mask, i);
        sigaction(i, &sigAction, NULL);
        sigaddset(&sigAction.sa_mask, i);
    }

    printf("Main process pid: %d\n\n", getpid());

    for (int i = 0; i < N ; ++i) {
        usleep(10);
        childProc = fork();
        if (childProc == 0){
            break;
        }
        DBGForkInfo(childProc);
    }

    if (childProc != 0) {
        while (requestCounter < K) pause();
        killpg(0, SIGUSR1);

        for (int j = 0; j < N; ++j) {
            if(DEBUG) printf("Waiting to exit %d\n", j);
            wait(NULL);
        }
    }

    if (childProc == 0){
        srand((unsigned int) getpid());

        sleepSec = (unsigned int) (rand() % 5);
        sleep((unsigned int) sleepSec);
        kill(getppid(), SIGUSR1);

        while (startPermission == 0) pause();
        kill(getppid(), (rand() % (SIGRTMAX-SIGRTMIN)) + SIGRTMIN);
        exit(sleepSec);

    }


    return 0;
}