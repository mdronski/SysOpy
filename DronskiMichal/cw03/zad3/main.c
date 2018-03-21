#include <stdio.h>
#include <malloc.h>
#include <zconf.h>
#include <memory.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>


void setLimits(char *cpuArg, char *memArg){
   long int cpuLimit = strtol(cpuArg, NULL, 10);
   long int memLimit = strtol(memArg, NULL, 10)*1000000;

   struct rlimit *cpuRLimit = calloc(1, sizeof(struct rlimit));
   struct rlimit *memRLimit = calloc(1, sizeof(struct rlimit));

    cpuRLimit->rlim_max = (rlim_t) cpuLimit;
    cpuRLimit->rlim_cur = (rlim_t) cpuLimit;
    memRLimit->rlim_max = (rlim_t) memLimit;
    memRLimit->rlim_cur = (rlim_t) memLimit;

    if(setrlimit(RLIMIT_CPU, cpuRLimit) == -1){
        printf("Unable to make cpu limit!\n");
    }
    if(setrlimit(RLIMIT_DATA, memRLimit) == -1){
        printf("Unable to make memory limit!\n");
    }
    if(setrlimit(RLIMIT_STACK, memRLimit) == -1){
        printf("Unable to make memory limit!\n");
    }
    free(cpuRLimit);
    free(memRLimit);

}

int main(int argc, char *argv[] ) {

    char *fileName = argv[1];
    FILE *fileHandler = fopen(fileName, "r");
    char singleLine[1024];
    char *args[128];
    int argsNumber = 0;
    int exitStatus;
    int pid = getpid();


    while(fgets(singleLine, 1024, fileHandler)){

        argsNumber = 0;
        args[argsNumber++] = strtok(singleLine, " \n");
        while ((args[argsNumber++] = strtok(NULL, " \n")) != NULL);

        pid_t childProcess = vfork();

        if (!childProcess){
            pid = getpid();

            printf("\nForked process with pid: %d\n\n", pid);
            setLimits(argv[2], argv[3]);
            printf("Limits set to: cpu: %ss  mem: %ldMb\n", argv[2], strtol(argv[3], NULL, 10));
            execvp(args[0], args);
        }
        wait(&exitStatus);
        printf("\nProces with pid: %d exited with status: %d \n", pid, exitStatus);
    }

    printf("\n");

    return 0;
}


