#include <stdio.h>
#include <sys/stat.h>
#include <zconf.h>

int main(int argc, char *argv[]) {

    char buffer[512];

    //mkfifo(argv[1], 0777);
    FILE *pipeReader = fopen(argv[1], "r");

    while (1){
        fgets(buffer,512 ,pipeReader);
        printf("%s", buffer);
//        sleep(2);
    }

//    for (int i = 0; i < 20; ++i) {
//        fgets(buffer,512 ,pipeReader);
//        printf("%s", buffer);
//        sleep(1);
//    }

    fclose(pipeReader);
    printf("ended\n");


    return 0;
}