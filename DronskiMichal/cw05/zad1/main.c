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

#define DESC_NUMB  10
int descryptors[DESC_NUMB][2];
int redirectionHanlders[DESC_NUMB];

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

void initPipes(){
    for(int i=0; i<DESC_NUMB; i++){
        if(pipe(descryptors[i]) == -1){
            perror("Pipe");
            exit(EXIT_FAILURE);
        }
    }
}

void closePipes(){
    for(int i=0; i<DESC_NUMB; i++){
        close(descryptors[i][0]);
        close(descryptors[i][1]);
        close(redirectionHanlders[i]);
    }
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
    char *args[512];
    char *commands[512];
    char buffer[4096];
    int argsNumber = 0;
    int commandNumber = 0;
    char *redirectionName[128];
    int redirected = 0;
    int wasRedirected = 0;
    pid_t childProcess;

    initPipes();


    while(fgets(singleLine, 1024, fileHandler)) {
        initPipes();

        if (strlen(singleLine) == 1) continue;
        printf("Command: %s", singleLine);
        commandNumber = 0;

        commands[commandNumber++] = strtok(singleLine, "|\n");
        while ((commands[commandNumber++] = strtok(NULL, "|\n")) != NULL);
        commandNumber--;


        for (int i = 0; i < commandNumber; ++i) {
            argsNumber = 0;

            if (strstr(commands[i], ">")){
                commands[i] = strtok(commands[i], ">");
                redirectionName[i] = strtok(NULL, ">");
                redirected = 1;
                redirectionHanlders[i] = open(redirectionName[i],  O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
            }

            args[argsNumber++] = strtok(commands[i], " \n");
            while ((args[argsNumber++] = strtok(NULL, " \n")) != NULL);

            argsNumber--;
            argsNumber = mergeArgs(args, argsNumber, '\"');
            argsNumber = mergeArgs(args, argsNumber, '\'');

            for (int i=0; i<argsNumber; i++) printf("%s ", args[i]);
            printf("\n");

            if ((childProcess = fork()) == -1){
                perror("Fork");
                exit(EXIT_FAILURE);
            }


            if (childProcess == 0) {

                if((i != commandNumber -1) || redirected)
                    dup2(descryptors[i+1][1],  STDOUT_FILENO);

                dup2(descryptors[i][0], STDIN_FILENO);

                if (execvp(args[0], args) == -1) {
                    perror("Error");
                    exit(-1);
                }
            }

            if (redirected){
                int length = (int) read(descryptors[i+1][0], buffer, 4096);
                write(redirectionHanlders[i], buffer, (size_t) length);
                write(descryptors[i+1][1], buffer, (size_t) length);
            }

            close(descryptors[i+1][1]);

            redirected = 0;
        }

        printf("All comands: %d, waiting for them to exit\n", commandNumber);
        for (int i = 0; i < commandNumber; ++i) {
            wait(NULL);
            printf("%d'th thread exited\n", i);
        }

        closePipes();
    }

    fclose(fileHandler);
    printf("\n");

    return 0;
}
