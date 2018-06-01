#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <zconf.h>
#include <uv.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/times.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

FILE *file;
FILE *filter_stream;
int height;
int width;
int c;
char buffer[128];
int **I;
int **I2;
double **K;
int thread_number;
pthread_t *threads;


int read_integer(FILE *stream){
    int n;
    fscanf(stream, "%d", &n);
    return n;
}

double read_double(FILE *stream){
    double n;
    fscanf(stream, "%lf", &n);
    return n;
}



int filter_pixel(int x, int y){
    double sum = 0;
    double a, b;
    for (int i = 0; i < c; ++i) {
        for (int j = 0; j < c; ++j) {
            a = min(height-1, x - ceil(c/2) + i);
            b = min(width-1, y - ceil(c/2) + j);
//            printf("a=%d  b=%d\n",(int) a, (int) b);
            sum += I[(int) max(0, a)][(int) max(0, b)] * K[i][j];
        }
    }
    if (sum < 0) return 0;
    return (int) round(sum);
}

void* filter_image_part(void *args){
    int *start_end = args;
    int start =  start_end[0];
    int end =  start_end[1];
    for (int i = start; i < end; ++i) {
        for (int j = 0; j < width; ++j) {
//            I2[i][j] = 255 - I[i][j];
            I2[i][j] = filter_pixel(i, j);
        }
    }
//    printf("Filtered image from %d to %d\n", start, end);
    return NULL;
}

void print_image(int **image){
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            printf("%d ", image[i][j]);
        }
        printf("\n");
    }
}

void print_filter(){
    for (int i = 0; i < c; ++i) {
        for (int j = 0; j < c; ++j) {
            printf("%lf ", K[i][j]);
        }
        printf("\n");
    }
}

void alloc_memory(){
    I = malloc(height * sizeof(int *));
    I2 = malloc(height * sizeof(int *));
    for (int i = 0; i < height; ++i) {
        I[i] = malloc(width * sizeof(int));
        I2[i] = malloc(width * sizeof(int));
    }

    K = malloc(c * sizeof(double *));
    for (int i = 0; i < c; ++i) {
        K[i] = malloc(c * sizeof(double));
    }

    threads = malloc(thread_number * sizeof(pthread_t));


}

void read_image(){
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            I[i][j] = read_integer(file);
        }
    }
}

void read_filter(){
    for (int i = 0; i < c; ++i) {
        for (int j = 0; j < c; ++j) {
            K[i][j] = read_double(filter_stream);
        }
    }
}

void save_filtered_image(char *output_filename){
    FILE *filtered_image = fopen(output_filename, "w+");
    fprintf(filtered_image, "P2\n%d %d\n%d\n", width, height, 255);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            fprintf(filtered_image, "%d ", I2[i][j]);
        }
        fprintf(filtered_image, "\n");
    }
    fclose(filtered_image);

}

void print_times(struct tms measurements[2], clock_t start, clock_t end){

    long int user_time = measurements[1].tms_utime - measurements[0].tms_utime;
    long int system_time = measurements[1].tms_stime - measurements[0].tms_stime;

    printf("Thread number: %d\nImage size: %d x %d\nFilter size: %d\n"
           "User time: %.2lf\nSystem time: %ld\nComputation time: %.2lf\n\n",
           thread_number, width, height, c,(double_t) user_time / 100, system_time, (double) (end - start) / 100);

}

int main(int argc, char **argv) {

    thread_number = atoi(argv[1]);
    char *input_filename = argv[2];
    char *filter_filename = argv[3];
    char *output_filename = argv[4];


    struct tms time_measurements[2];
    file = fopen(input_filename, "r");
    filter_stream = fopen(filter_filename, "r");

    fgets(buffer, 100, file);
    width = read_integer(file);
    height = read_integer(file);
    read_integer(file);
    c = read_integer(filter_stream);

    alloc_memory();

    read_image();

    read_filter();

    clock_t start = times(&time_measurements[0]);

    for (int i = 0; i < thread_number; ++i) {
        int *args = calloc(2 , sizeof(int));
        args[0] = i * (width / thread_number);
        args[1] = (i+1) * (width / thread_number);
        if(i == thread_number-1){
            args[1] = width;
        }
        pthread_create(&(threads[i]), NULL, filter_image_part, args);
    }

    for (int j = 0; j < thread_number; j++) {
        if(pthread_join(threads[j], NULL)){
            perror("Error while joining threads ");
        }
    }

    clock_t end = times(&time_measurements[1]);

    print_times(time_measurements, start, end);

    save_filtered_image(output_filename);



    free(I);
    free(I2);
    free(K);
    fclose(filter_stream);
    fclose(file);


    return 0;
}