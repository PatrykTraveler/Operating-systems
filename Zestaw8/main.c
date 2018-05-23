#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define MAX_LINE 4096
#define MAX_ARGS 1000

#define EXIT_ON_ERROR(format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(EXIT_FAILURE);}

int *picture_arr, *result_arr;
float *filter_arr;

int W, H, C, C_scale, threads;

void pgm_parser(FILE* file){
    int i = 0;
    char* buff, *rest;
    char buffer[MAX_LINE];
    int arguments[MAX_ARGS];
    int arg_counter = 0;

    fgets(buffer, MAX_LINE, file);
    rest = buffer;
    C_scale = (int)strtol(strtok_r(NULL, " \n", &rest), NULL, 10);
    fgets(buffer, MAX_LINE, file);
    rest = buffer;
    while((buff = strtok_r(NULL, " \n", &rest))) {
        arguments[arg_counter++] = (int)strtol(buff, NULL, 10);
    }
    W = arguments[0];
    H = arguments[1];
    picture_arr = calloc(W*H, sizeof(int));
    fgets(buffer, MAX_LINE, file);
    while(fgets(buffer, MAX_LINE, file)){
        rest = buffer;
        while((buff = strtok_r(NULL, " \n", &rest))) {
            picture_arr[i++] = (int)strtol(buff, NULL, 10);
        }
    }
}

void filter_parser(FILE* file){
    int i = 0;
    char *rest, *buff;
    char line[MAX_LINE];

    fgets(line, MAX_LINE, file);
    rest = line;
    C = (int)strtol(strtok_r(NULL, " \n", &rest), NULL, 10);
    filter_arr = calloc(C*C, sizeof(float));

    while(fgets(line, MAX_LINE, file)) {
        rest = line;
        while ((buff = strtok_r(NULL, " \n", &rest))) {
            filter_arr[i++] = strtof(buff, NULL);
        }
    }
}

void *filter_array(void *args){
    int c2 = (int)floor(C/2);
    int thread_number = *((int*)args);
    printf("%d", threads);
    int start_row = H*thread_number/threads;
    int end_row = H*(thread_number + 1)/threads;
    double s;

    for(int y = start_row; y < end_row; y++){
        for(int x = 0; x < W; x++){
            s = 0;
            for(int i = 0; i < C; i++){
                for(int j = 0; j < C; j++){
                    int px = (int)fmax(1, x - c2 + i);
                    int py = (int)fmax(1, y - c2 + j);
                    s += picture_arr[py * W + px]*filter_arr[i*C + j];
                }
            }
            result_arr[y*W + x] = (int)roundf(s);
        }
    }
    return NULL;
}

void save_file(FILE* file){
    fprintf(file, "P2\n");
    fprintf(file, "%d %d\n", W, H);
    fprintf(file, "%d\n", C_scale);
    for(int i = 0; i < H; i++){
        for(int j = 0; j < W; j++){
            fprintf(file, "%d ", result_arr[i*W + j]);
        }
        fprintf(file, "\n");
    }
}

int main(int argc, char *argv[]) {
    if(argc < 5)
        EXIT_ON_ERROR("Insufficient number of arguments!\n");

    int threads = (int)strtol(argv[1], NULL, 10);
    char* picture_path = argv[2];
    char* filter_path = argv[3];
    char* output_file = argv[4];

    FILE* file1 = fopen(picture_path, "r+");
    FILE* file2 = fopen(filter_path, "r+");
    FILE* file3 = fopen(output_file, "w+");
    if(file1 == NULL || file2 == NULL)
        return 0;

    pgm_parser(file1);
    filter_parser(file2);

    result_arr = calloc(W*H, sizeof(int));

    pthread_t* threads_array = malloc(threads * sizeof(pthread_t));
    for(int i = 0; i < threads; i++){
        int *arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&threads_array[i], NULL, filter_array, arg);
    }
    for(int i = 0; i < threads; i++){
        void *p;
        pthread_join(threads_array[i], &p);
    }

    save_file(file3);
    fclose(file1);
    fclose(file2);
    fclose(file3);
    return 0;
}