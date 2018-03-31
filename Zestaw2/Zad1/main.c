#define _DEFAULT_SOURCE

#define STR_EQUAL(x, y) strcmp(x, y) == 0
#define toInt(x) (int)strtol(x, NULL, 10)

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <memory.h>
#include "libusage.h"
#include "sysusage.h"


void show(const char* f1, const char* f2, int rnumber, size_t rsize){
    FILE* f = fopen(f1, "r");
    FILE* new = fopen(f2, "w");

    unsigned char* buffer = calloc(rsize, sizeof(unsigned char));
    unsigned char* wspace = calloc(1, sizeof(unsigned char));
    wspace[0] = '\n';
    for(int i = 0; i < rnumber; i++){
        fread(buffer, sizeof(unsigned char), rsize, f);
        fwrite(buffer, sizeof(unsigned char), rsize, new);
        fputc(10, new);
    }
    free(buffer);
    free(wspace);
    fclose(f);
    fclose(new);
}

//tests

struct timeval calcTime(struct timeval t1, struct timeval t2){
    struct timeval newTime;
    timersub(&t2, &t1, &newTime);
    return newTime;
}

void copyTest(
        const char* filename,
        int rnumber,
        size_t rsize,
        void (*generate)(const char*, int, size_t),
        void (*copy)(const char*, const char*, int, size_t),
        const char* mode
){
    struct timeval t1, t2, user_start, user_end, system_start, system_end;
    struct rusage usage;

    generate(filename, rnumber, rsize);

    getrusage(RUSAGE_SELF, &usage);
    system_start = usage.ru_stime;
    user_start = usage.ru_utime;
    gettimeofday(&t1, 0);

    copy(filename, "backup", rnumber, rsize);

    gettimeofday(&t2, 0);
    getrusage(RUSAGE_SELF, &usage);
    system_end = usage.ru_stime;
    user_end = usage.ru_utime;

    printf("\nCopy test: \n------------------------------\n");
    printf("MODE: %s                RECORD NUMBER: %d                RECORD SIZE: %d\n", mode, rnumber, rsize);
    printf("Real time: %ld.%lds\n", calcTime(t1,t2).tv_sec, calcTime(t1,t2).tv_usec);
    printf("User time: %ld.%lds\n", calcTime(user_start, user_end).tv_sec, calcTime(user_start, user_end).tv_usec);
    printf("System time: %ld.%lds\n", calcTime(system_start, system_end).tv_sec, calcTime(system_start, system_end).tv_usec);
}

void sortTest(
        const char* filename,
        int rnumber,
        size_t rsize,
        void (*generate)(const char*, int, size_t),
        void (*sort)(const char*, int, size_t),
        void (*copy)(const char*, const char*, int, size_t),
        const char* mode
){
    struct timeval t1, t2, user_start, user_end, system_start, system_end;
    struct rusage usage;

    generate(filename, rnumber, rsize);
    copy(filename, "backup", rnumber, rsize);

    getrusage(RUSAGE_SELF, &usage);
    system_start = usage.ru_stime;
    user_start = usage.ru_utime;
    gettimeofday(&t1, 0);

    sort(filename, rnumber, rsize);

    gettimeofday(&t2, 0);
    getrusage(RUSAGE_SELF, &usage);
    system_end = usage.ru_stime;
    user_end = usage.ru_utime;

    printf("\nSort test: \n------------------------------\n");
    printf("MODE: %s                RECORD NUMBER: %d                RECORD SIZE: %d\n", mode, rnumber, rsize);
    printf("Real time: %ld.%lds\n", calcTime(t1,t2).tv_sec, calcTime(t1,t2).tv_usec);
    printf("User time: %ld.%lds\n", calcTime(user_start, user_end).tv_sec, calcTime(user_start, user_end).tv_usec);
    printf("System time: %ld.%lds\n", calcTime(system_start, system_end).tv_sec, calcTime(system_start, system_end).tv_usec);
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    if(argc < 6){
        printf("You have to provide at least 5 parameters\n");
	exit(-1);
    }

    int rnumber, rsize;
    char *file1, *mode;

    mode = argv[argc-1];

    void (*generate)(const char*, int, size_t) = &libgenerate;
    void (*sort)(const char*, int, size_t) = STR_EQUAL(mode, "lib") ? &libsort : &syssort;
    void (*copy)(const char*, const char*, int, size_t) = STR_EQUAL(mode, "lib") ? &libcopy : &syscopy;


    if(STR_EQUAL(argv[1], "generate")){
        file1 = argv[2];
        rnumber = toInt(argv[3]);
        rsize = toInt(argv[4]);

        generate(file1, rnumber, (size_t)rsize);
    }
    else if(STR_EQUAL(argv[1], "sort")){
        file1 = argv[2];
        rnumber = toInt(argv[3]);
        rsize = toInt(argv[4]);

        sort(file1, rnumber,(size_t)rsize);
    }
    else if(STR_EQUAL(argv[1], "copy")){
        char* file2;

        file1 = argv[2];
        file2 = argv[3];

        rnumber = toInt(argv[4]);
        rsize = toInt(argv[5]);

        copy(file1, file2, rnumber, (size_t)rsize);
    }
    else if(STR_EQUAL(argv[1], "test")) {
        file1 = argv[2];
        rnumber = toInt(argv[3]);
        rsize = toInt(argv[4]);

        copyTest(file1, rnumber, rsize, generate, copy, mode);
        sortTest(file1, rnumber, rsize, generate, sort, copy, mode);
    }
    else {
        printf("Wrong parameters.");
        exit(-1);
    }

    return 0;
}


