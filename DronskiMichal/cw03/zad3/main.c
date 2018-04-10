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

void shuffle(char* args[128], int currentPos, int argsNumber){
    while (currentPos+1 < argsNumber){
        args[currentPos] = args[currentPos+1];
        currentPos ++;
    }
    args[currentPos] = NULL;
}

int mergeArgs(char* args[128], int argsNumber, int charPattern){
   int i = 0;
   int merged = 0;
   while(i+1 < argsNumber-merged){
       if (args[i][0] == charPattern){
           do {
               args[i][strlen(args[i])] = ' ';
               shuffle(args, i+1, argsNumber-merged);
               merged ++;
           } while ((i < argsNumber-merged) && ((args[i][strlen(args[i])-1] != charPattern ) ||
                     (args[i][strlen(args[i])-1] == charPattern && args[i][strlen(args[i])-2] == '\\')));
           args[i][strlen(args[i])-1] = 0;
           args[i] = &args[i][1];
       }
       i ++;
   }
    return argsNumber - merged;
}

void setLimits(char *cpuArg, char *memArg){
    struct rlimit cpuRLimit;
    struct rlimit memRLimit;
    long int cpuLimit = strtol(cpuArg, NULL, 10);
    long int memLimit = strtol(memArg, NULL, 10)*1024*1024;

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

    long int maxMem = childUsage[1].ru_maxrss;

    printf("\nSystem time: %lf\n", sysTime);
    printf("User   time: %lf\n", usrTime);
    printf("Max  Memory: %ld\n", maxMem);
    printf("-----------------------------------------------------------\n");

}

int main(int argc, char *argv[] ) {

    if (argc < 4){
        printf("Wrong arguments! Needs file with executables, CPU and Memory limits!\n");
        return (-1);
    }


    char *fileName = argv[1];
    FILE *fileHandler = fopen(fileName, "r");

    if (fileHandler == 0){
        perror("Opening file");
        exit(0);
    }

    char singleLine[1024];
    char *args[128];
    int argsNumber = 0;
    int exitStatus;

    int pid = getpid();
    struct rusage childUsage[2];


    while(fgets(singleLine, 1024, fileHandler)){

        if (strlen(singleLine) == 1) continue;
        printf("Command: %s", singleLine);
        argsNumber = 0;
        args[argsNumber++] = strtok(singleLine, " \n");
        while ((args[argsNumber++] = strtok(NULL, " \n")) != NULL);

        argsNumber--;
        argsNumber = mergeArgs(args, argsNumber, '\"');
        argsNumber = mergeArgs(args, argsNumber, '\'');

        getrusage(RUSAGE_CHILDREN, &childUsage[0]);

        pid_t childProcess = vfork();

        if (!childProcess){
            pid = getpid();

            printf("\nForked process with pid: %d\n", pid);
            setLimits(argv[2], argv[3]);
            printf("Limits set to: cpu: %ss  mem: %ldMib\n\n", argv[2], strtol(argv[3], NULL, 10));
            if(execvp(args[0], args) == -1){
                perror("Error");
                exit(-1);
            }
        }

        wait(&exitStatus);
        if (exitStatus > 0){
            printf("Program failed on \"%s\" with exit code: %d\nAborting...\n", args[0] , exitStatus);
            exit(0);
        }
        getrusage(RUSAGE_CHILDREN, &childUsage[1]);
        printf("\nProces with pid: %d exited with status: %d \n", pid, exitStatus);
        printRusage(childUsage);

    }
    fclose(fileHandler);
    printf("\n");

    return 0;
}
