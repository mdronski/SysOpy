#include <stdio.h>
#include <sys/stat.h>
#include <zconf.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    int N = 20;
    char buffer[512];

    FILE *pipeWriter;
    FILE *dataReader;




    for (int i = 0; i < N; ++i) {
        dataReader = popen("date", "r");
        pipeWriter = fopen(argv[1], "w");
        fgets(buffer, 512, dataReader);
        printf("%d - %s\n", i, buffer);
        fwrite(buffer, sizeof(char), (size_t) 512, pipeWriter);
        fclose(pipeWriter);
        fclose(dataReader);
        sleep(1);
    }

    printf("endedSlave\n");


    return 0;
}