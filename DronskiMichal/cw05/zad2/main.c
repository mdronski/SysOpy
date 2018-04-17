#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <zconf.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>


static void intAction(int sigNum, siginfo_t* info, void* vp){
    killpg(0, SIGINT);
    exit(EXIT_SUCCESS);
}

static void rtAction(int sigNum, siginfo_t* info, void* vp){
}



int childPid;
int masterPid;
int childPids[4096];


int main(int argc, char *argv[]) {

    char buffer[512];
    int slaveNumber;
    int N;
    struct sigaction sigAction;


    if (argc != 4){
        printf("Wrong main arguments!\n");
        exit(EXIT_FAILURE);
    }

    sigfillset(&sigAction.sa_mask);
    sigAction.sa_flags = SA_SIGINFO;
    sigAction.sa_sigaction = &intAction;
    sigaction(SIGINT, &sigAction, NULL);

    sigAction.sa_sigaction = &rtAction;
    sigaction(SIGRTMIN+1, &sigAction, NULL);

    slaveNumber = (int) strtol(argv[2], NULL, 10);
    N = (int) strtol(argv[3], NULL, 10);

    masterPid = fork();
    if (masterPid == 0){
        execlp("./master", "master", "myFifo", 0);
    }

    pause();

    for (int i = 0; i < slaveNumber; ++i) {
        childPids[i] = fork();
        if (childPids[i] == 0){
            execlp("./slave","slave", argv[1], argv[3], 0);
        }
    }


    for (int i = 0; i < slaveNumber; ++i) {
        waitpid(childPids[i], NULL, WUNTRACED);
        //printf("%d'th slave exited\n", i);
    }

//    kill(masterPid, SIGINT);
    killpg(0, SIGINT);

    waitpid(masterPid, NULL, WUNTRACED);

    return 0;
}
