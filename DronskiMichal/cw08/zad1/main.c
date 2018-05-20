#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define M 255

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

int filter(int x, int y){
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

void alloc_images_mem(){
    I = malloc(height * sizeof(int *));
    I2 = malloc(height * sizeof(int *));
    for (int i = 0; i < height; ++i) {
        I[i] = malloc(width * sizeof(int));
        I2[i] = malloc(width * sizeof(int));
    }
}

void read_image(){
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            I[i][j] = read_integer(file);
        }
    }
}

void alloc_filter_mem(){
    K = malloc(c * sizeof(double *));
    for (int i = 0; i < c; ++i) {
        K[i] = malloc(c * sizeof(double));
    }
}

void read_filter(){
    for (int i = 0; i < c; ++i) {
        for (int j = 0; j < c; ++j) {
            K[i][j] = read_double(filter_stream);
        }
    }
}

void save_filtered_image(){
    FILE *filtered_image = fopen("filtered_image.pgm", "w+");
    fprintf(filtered_image, "P2\n%d %d\n%d\n", width, height, 255);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            fprintf(filtered_image, "%d ", I2[i][j]);
        }
        fprintf(filtered_image, "\n");
    }
    fclose(filtered_image);

}


int main(int argc, char **argv) {


    file = fopen("lena.ascii.pgm", "r");
    fgets(buffer, 100, file);

    width = read_integer(file);
    height = read_integer(file);
    read_integer(file);

    alloc_images_mem();

    read_image();

    filter_stream = fopen("filter.txt", "r");

    c = read_integer(filter_stream);

    alloc_filter_mem();

    read_filter();




    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            I2[i][j] = filter(i, j);
        }
    }


    save_filtered_image();

    fclose(filter_stream);
    fclose(file);


    return 0;
}