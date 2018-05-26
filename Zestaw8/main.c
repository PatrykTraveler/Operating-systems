#define _DEFAULT_SOURCE
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_LINE 4096
#define MAX_ARGS 1000

#define EXIT_ON_ERROR(format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(EXIT_FAILURE);}
#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(c, d) \
   ({ __typeof__ (c) _c = (c); \
       __typeof__ (d) _d = (d); \
_c > _d ? _d : _c; })

int *picture_arr, *result_arr;
float *filter_arr;

int W, H, C, C_scale, threads;

struct timeval calcTime(struct timeval t1, struct timeval t2){
    struct timeval newTime;
    timersub(&t2, &t1, &newTime);
    return newTime;
}

void saveTimes(struct timeval tv[2]){
    struct timeval result = calcTime(tv[0], tv[1]);
    FILE* file = fopen("Times.txt", "a+");
    fprintf(file, "RESOLUTION: %d X %d    FILTER C: %d    THREADS: %d", W, H, C, threads);
    fprintf(file, "\nTIME: %ld.%lds\n\n", result.tv_sec, result.tv_usec);
    fclose(file);
}

void pgm_parser(FILE* file){
    int i = 0;
    char* buff, *rest;
    char buffer[MAX_LINE];
    int arguments[MAX_ARGS];
    int arg_counter = 0;

    fgets(buffer, MAX_LINE, file);
    fgets(buffer, MAX_LINE, file);
    rest = buffer;
    while((buff = strtok_r(NULL, " \n\r", &rest))) {
        arguments[arg_counter++] = (int)strtol(buff, NULL, 10);
    }
    W = arguments[0];
    H = arguments[1];
    picture_arr = calloc(W*H, sizeof(int));
    fgets(buffer, MAX_LINE, file);
    rest = buffer;
    C_scale = (int)strtol(strtok_r(NULL, " \n\r", &rest), NULL, 10);
    while(fgets(buffer, MAX_LINE, file)){
        rest = buffer;
        while((buff = strtok_r(NULL, " \n\r", &rest))) {
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
    C = (int)strtol(strtok_r(NULL, " \n\r", &rest), NULL, 10);
    filter_arr = calloc(C*C, sizeof(float));

    while(fgets(line, MAX_LINE, file)) {
        rest = line;
        while ((buff = strtok_r(NULL, " \n\r", &rest))) {
            filter_arr[i++] = strtof(buff, NULL);
        }
    }
}

void *filter_array(void *args){
    int c2 = (int)ceil(C/2);
    int thread_number = *((int*)args);
    int start_row = H*thread_number/threads;
    int end_row = H*(thread_number + 1)/threads;
    double s;
    for(int y = start_row; y < end_row; y++){
        for(int x = 0; x < W; x++){
            s = 0;
            for(int i = 0; i < C; i++){
                for(int j = 0; j < C; j++){
                    int py = min((H-1), max(0, y - c2 + i));
                    int px = min((W-1), max(0, x - c2 + j));
                    s += picture_arr[py * W + px]*filter_arr[i*C + j];
                }
            }
            result_arr[y*W + x] = (int)round(s);
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

    threads = (int)strtol(argv[1], NULL, 10);
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

    struct timeval tv[2];
    gettimeofday(&tv[0], 0);

    for(int i = 0; i < threads; i++){
        int *arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&threads_array[i], NULL, filter_array, arg);
    }

    for(int i = 0; i < threads; i++){
        void *p;
        pthread_join(threads_array[i], &p);
    }
    gettimeofday(&tv[1], 0);
    saveTimes(tv);

    save_file(file3);
    fclose(file1);
    fclose(file2);
    fclose(file3);
    return 0;
}
