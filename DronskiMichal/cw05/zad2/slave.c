#include <stdio.h>
#include <sys/stat.h>
#include <zconf.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

void buildResult(char result[], char pid[], char date[]){
    strcat(result, "Pid: ");
    strcat(result, pid);
    strcat(result, " - ");
    strcat(result, date);

}

int main(int argc, char *argv[]) {
    srand( (unsigned) time(NULL));
    //srand( time(NULL));
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


    for (int i = 0; i < N; ++i) {
        sleep((unsigned int) ((rand() + pid) % 10) );
        dataReader = popen("date", "r");
        pipeWriter = open(argv[1], O_WRONLY);
        fgets(dateBuffer, 512, dataReader);

        buildResult(resultBuffer, pidBuffer, dateBuffer);

        write(pipeWriter, resultBuffer, (size_t) strlen(resultBuffer));
        close(pipeWriter);
        fclose(dataReader);

        memset(resultBuffer, 0, 512);
        //srand( (unsigned) time(NULL));
    }

    printf("returning from slave with pid: %d\n", pid);



    return 0;
}