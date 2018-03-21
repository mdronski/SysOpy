#include <stdio.h>
#include <malloc.h>
#include <zconf.h>
#include <memory.h>
#include <sys/wait.h>
#include <stdlib.h>


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
            execvp(args[0], args);
        }
        wait(&exitStatus);
        printf("\nProces with pid: %d exited with status: %d \n", pid, exitStatus);
    }

    printf("\n");

    return 0;
}