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
   long int memLimit = strtol(memArg, NULL, 10);

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

}

int main(int argc, char *argv[] ) {

    char *fileName = argv[1];
    FILE *fileHandler = fopen(fileName, "r");
    long int cpuTime = strtol(argv[2], NULL, 10);
    long int virtualMem = strtol(argv[3], NULL, 10);
    char singleLine[1024];// = calloc(128, sizeof(char));
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
            printf("Limits set to: cpu: %ss  mem: %.2lfMb\n", argv[2], (double) strtol(argv[3], NULL, 10) / 1000000);
            execvp(args[0], args);
        }
        wait(&exitStatus);
        printf("\nProces with pid: %d exited with status: %d \n", pid, exitStatus);
        //
    }

    printf("\n");

    return 0;
}



//char **args = calloc( (size_t) numberOfWords, sizeof(char*));
//for (int i = 0; i < numberOfWords; ++i) {
//args[i] = calloc(128, sizeof(char));
//}
//strcpy(buffer, singleLine);
//
//command = strtok(buffer, " ");
//
//word = strtok(NULL, " ");
//args[0] = command;
//
//for (int i = 1; i < numberOfWords-1; ++i) {
//strcpy(args[i], word);
//word = strtok(NULL, " \n");
//}
//args[numberOfWords-2][strlen(args[numberOfWords-2])-1] = '\0';
//args[numberOfWords-1] = NULL;
//
//numberOfWords = 0;
//
//pid_t childProcess = vfork();
//if (!childProcess){
//pid = getpid();
//
//cpuLimit->rlim_max = (rlim_t) cpuTime;
//cpuLimit->rlim_cur = (rlim_t) cpuTime;
//vMemLimit->rlim_max = (rlim_t) virtualMem;
//vMemLimit->rlim_cur = (rlim_t) virtualMem;
//if(setrlimit(RLIMIT_CPU, cpuLimit) == -1){
//printf("Unable to make cpu limit!\n");
//}
//if(setrlimit(RLIMIT_DATA, vMemLimit) == -1){
//printf("Unable to make memory limit!\n");
//}
//if(setrlimit(RLIMIT_STACK, vMemLimit) == -1){
//printf("Unable to make memory limit!\n");
//}
//
//
//printf("\nForked process with pid: %d\n\n", pid);
//execvp(command, args);
////            printf("\nProces with pid: %d exited with status %d", pid, errno);
//}
//wait(&exitStatus);
////        perror("command: ");
//printf("\nProces with pid: %d exited with status: %d \n", pid, exitStatus);
////
//}
