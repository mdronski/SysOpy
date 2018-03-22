#include <stdio.h>
#include <malloc.h>
#include <zconf.h>
#include <memory.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <fcntl.h>


void setLimits(char *cpuArg, char *memArg){
   long int cpuLimit = strtol(cpuArg, NULL, 10);
   long int memLimit = strtol(memArg, NULL, 10)*1024*1024;

   struct rlimit cpuRLimit;
   struct rlimit memRLimit;

    cpuRLimit.rlim_max = (rlim_t) cpuLimit;
    cpuRLimit.rlim_cur = (rlim_t) cpuLimit;
    memRLimit.rlim_max = (rlim_t) memLimit;
    memRLimit.rlim_cur = (rlim_t) memLimit;

    if(setrlimit(RLIMIT_CPU, &cpuRLimit) == -1){
        printf("Unable to make cpu limit!\n");
    }
    if(setrlimit(RLIMIT_DATA, &memRLimit) == -1){
        printf("Unable to make memory limit!\n");
    }
    if(setrlimit(RLIMIT_STACK, &memRLimit) == -1){
        printf("Unable to make memory limit!\n");
    }
}

void printRusage(struct rusage childUsage[2]){
    long int sysTimeStart = childUsage[0].ru_stime.tv_sec*1000000 + childUsage[0].ru_stime.tv_usec;
    long int sysTimeEnd = childUsage[1].ru_stime.tv_sec*1000000 + childUsage[1].ru_stime.tv_usec;
    double sysTime = (double) (sysTimeEnd - sysTimeStart) / 1000000;

    long int usrTimeStart = childUsage[0].ru_utime.tv_sec*1000000 + childUsage[0].ru_utime.tv_usec;
    long int usrTimeEnd = childUsage[1].ru_utime.tv_sec*1000000 + childUsage[1].ru_utime.tv_usec;
    double usrTime = (double) (usrTimeEnd - usrTimeStart) / 1000000;

    long int stackMemStart = childUsage[0].ru_isrss;
    long int stackMemEnd = childUsage[1].ru_isrss;
    double stackMem = (double) (stackMemEnd); //- stackMemStart) / (1024*1024);

    long int dataStart = childUsage[0].ru_idrss;
    long int dataEnd = childUsage[1].ru_idrss;
    double data = (double) (dataEnd);// - dataStart) / (1024*1024);

    long int sharedMem = childUsage[1].ru_maxrss;

    printf("\nSystem time: %lf\n", sysTime);
    printf("User   time: %lf\n", usrTime);
    printf("Max  Memory: %ld\n", sharedMem);
    printf("-----------------------------------------------------------\n");

}

int main(int argc, char *argv[] ) {

    char *fileName = argv[1];
    FILE *fileHandler = fopen(fileName, "r");
    char singleLine[1024];
    char *args[128];
    int argsNumber = 0;
    int exitStatus;
    int wstatus = -1;
    int pid = getpid();
    struct rusage childUsage[2];

    while(fgets(singleLine, 1024, fileHandler)){

        argsNumber = 0;
        args[argsNumber++] = strtok(singleLine, " \n");
        while ((args[argsNumber++] = strtok(NULL, " \n")) != NULL);

        getrusage(RUSAGE_CHILDREN, &childUsage[0]);

        pid_t childProcess = vfork();

        if (!childProcess){
            pid = getpid();

            printf("\nForked process with pid: %d\n", pid);
            setLimits(argv[2], argv[3]);
            printf("Limits set to: cpu: %ss  mem: %ldMb\n\n", argv[2], strtol(argv[3], NULL, 10));
            execvp(args[0], args);
        }

        wait(&exitStatus);
        getrusage(RUSAGE_CHILDREN, &childUsage[1]);
        printf("\nProces with pid: %d exited with status: %d \n", pid, exitStatus);
        printRusage(childUsage);

    }

    printf("\n");

    return 0;
}


