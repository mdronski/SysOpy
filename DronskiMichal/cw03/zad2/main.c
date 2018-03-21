#include <stdio.h>
#include <malloc.h>
#include <zconf.h>
#include <memory.h>
#include <sys/wait.h>


int main(int argc, char *argv[] ) {

    char *fileName = argv[1];
    FILE *fileHandler = fopen(fileName, "r");
    char singleLine[1024];// = calloc(128, sizeof(char));
    char buffer[1024];
    char* word;
    int numberOfWords = 0;
    char* command;
    int exitStatus = 0;
    int pid = getpid();
    char *tab[3] = {"ls", "-l", NULL};

//    execlp("ls", "ls", "-l", NULL);


    while(fgets(singleLine, 1024, fileHandler)){
        strcpy(buffer, singleLine);
        strtok(buffer, " ");
        while(strtok(NULL, " ")) numberOfWords ++;
        numberOfWords++;
        numberOfWords++;

        char **args = calloc( (size_t) numberOfWords, sizeof(char*));
        for (int i = 0; i < numberOfWords; ++i) {
            args[i] = calloc(128, sizeof(char));
        }
        strcpy(buffer, singleLine);

        command = strtok(buffer, " ");

        word = strtok(NULL, " ");
        args[0] = command;

        for (int i = 1; i < numberOfWords-1; ++i) {
            strcpy(args[i], word);
            word = strtok(NULL, " ");
        }
        args[numberOfWords-2][strlen(args[numberOfWords-2])-1] = '\0';
        args[numberOfWords-1] = NULL;
//
//        for (int j = 0; j < numberOfWords-1; ++j) {
//             printf("%d ", j);
//             printf("%s\n", args[j]);
//        }
//        if (args[numberOfWords] == NULL) printf("%d Null\n", numberOfWords-1);
//        printf("%s\n", command);
        numberOfWords = 0;

        pid_t childProcess = vfork();
        if (!childProcess){
            pid = getpid();
            printf("\nForked process with pid: %d\n\n", pid);
        //    execlp("/home/mdronski/SO/DronskiMichal/cw02/zad2/nftwLS","nftwLS" "-p",".","-c",">","-y","2000","-M","10","-d","10","-h","10","-m","10", NULL);
              execvp(command, args);
//            execvp("ls", tab);
        }
        wait(&exitStatus);
        printf("\nProces with pid: %d exited with status %d", pid, exitStatus);
    }

    printf("\n");

    return 0;
}