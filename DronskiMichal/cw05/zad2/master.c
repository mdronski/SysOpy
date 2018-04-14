#include <stdio.h>
#include <sys/stat.h>
#include <zconf.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

FILE *pipeReader;

static void intAction(int sigNum, siginfo_t* info, void* vp){
    printf("returning from master\n");
    fclose(pipeReader);
    exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[]) {

    size_t bufferSize = 512;
    char buffer[bufferSize];
    char *myBuffer = (char *) malloc(bufferSize + 1);

    struct sigaction sigAction;
    sigfillset(&sigAction.sa_mask);
    sigAction.sa_flags = SA_SIGINFO;
    sigAction.sa_sigaction = &intAction;
    sigaction(SIGINT, &sigAction, NULL);

    pipeReader = fopen(argv[1], "r");

    while (1){
        if(getline(&myBuffer, &bufferSize ,pipeReader) >= 0)
        printf("%s", myBuffer);

    }

}