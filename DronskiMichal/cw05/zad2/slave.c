#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <zconf.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    srand( (unsigned) time(NULL));
    if (argc != 3){
        printf("Wrong slave arguments!\n");
        exit(EXIT_FAILURE);
    }

    int N;
    int pid;
    char dateBuffer[512];
    char pidBuffer[128];
    char resultBuffer[512];

    int pipeWriter;
    FILE *dataReader;


    N = (int) strtol(argv[2], NULL, 10);
    pid = getpid();
    sprintf(pidBuffer, "%d", pid);
    pipeWriter = open(argv[1], O_WRONLY);


    for (int i = 0; i < N; ++i) {
        dataReader = popen("date", "r");
        fgets(dateBuffer, 512, dataReader);
        sprintf(resultBuffer,"Pid: %s - %s", pidBuffer, dateBuffer);
        write(pipeWriter, resultBuffer, (size_t) strlen(resultBuffer));
        fclose(dataReader);
        sleep((unsigned int) ((rand() + pid) % 2) );

    }

    close(pipeWriter);
    //printf("returning from slave with pid: %d\n", pid);



    return 0;
}