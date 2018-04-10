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

int main(int argc, char *argv[] ) {

    if (argc < 2){
        printf("Wrong arguments! Needs file with executables!\n");
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

    while(fgets(singleLine, 1024, fileHandler)){

        if (strlen(singleLine) == 1) continue;
        printf("Command: %s", singleLine);
        argsNumber = 0;
        args[argsNumber++] = strtok(singleLine, " \n");
        while ((args[argsNumber++] = strtok(NULL, " \n")) != NULL);

        argsNumber--;
        argsNumber = mergeArgs(args, argsNumber, '\"');
        argsNumber = mergeArgs(args, argsNumber, '\'');

        pid_t childProcess = vfork();

        if (!childProcess){
            pid = getpid();

            printf("\nForked process with pid: %d\n", pid);
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
        printf("\nProces with pid: %d exited with status: %d \n", pid, exitStatus);

    }
    fclose(fileHandler);
    printf("\n");

    return 0;
}
