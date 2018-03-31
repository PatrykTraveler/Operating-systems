#define _DEFAULT_SOURCE
#define MAX_SIZE 1000
#define BLOCK_SIZE 16
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <memory.h>
#include <dlfcn.h>

#ifndef DLL
#include "library.h"
#endif

#ifdef DLL
typedef char char_t[BLOCK_SIZE];

typedef struct block_t{
    union {
        char_t *static_ptr;
        char **dynamic_ptr;
    };

    unsigned int max_size;
    unsigned int block_size;
    unsigned int is_dynamic;
}block_t;

    block_t* (*set_array)(int, int, int);
    void (*remove_array)(block_t*);
    void (*set_block)(block_t*, const char*, int);
    void (*remove_block)(block_t*, int);
    int (*find_block)(block_t*, const char*);
#endif

const char* program_name;

void print_usage(FILE* stream, int exit_code){
    fprintf(stream, "Usage: %s\n", program_name);
    fprintf(stream,
            "-h                --help            Display this usage information. \n"
            "-s                --size            Set size of an array\n"
            "-b                --block_size      Set blocks size\n"
            "-m                --mode            Set allocation mode, 0 - static, 1 - dynamic\n"
            "-c                --create          Create array\n"
            "-f                --find_element    Find the most similiar element to a given one\n"
            "-a                --add             Add specfic number of blocks\n"
            "-r                --remove_and_add  Remove and add specific number of blocks\n");
    exit(exit_code);
}

const char* generate_string(int max_size){
    if(max_size < 1)
        return NULL;

    char* seed = "ABCDEFGHIJKLMNOPRSTWYZabcdefghijklmnoprstwy";
    size_t seed_length = strlen(seed);
    char* buff = malloc(max_size*sizeof(char));
    int buff_length = max_size - rand() % (max_size - 1);

    for(int i = 0; i < buff_length - 1; i++)
        buff[i] = seed[rand() % seed_length];
    return buff;
}

void generateBlocks(block_t* block){
#ifdef DLL
    void *libPtr = dlopen("./liblibrary.so", RTLD_LAZY);
    void (*set_block)(block_t*, const char*, int) = dlsym(libPtr,"set_block");
#endif

    for(int i = 0; i < block->max_size; i++)
        set_block(block, generate_string(block->block_size), i);
}

void addBlocks(block_t* block, int number){
#ifdef DLL
    void *libPtr = dlopen("./liblibrary.so", RTLD_LAZY);
    void (*set_block)(block_t*, const char*, int) = dlsym(libPtr,"set_block");
#endif

    if(number > block->max_size)
        return;

    for(int i = 0; i < number; i++)
        set_block(block, generate_string(block->block_size), i);
}

void removeBlocks(block_t* block, int number){
#ifdef DLL
    void *libPtr = dlopen("./liblibrary.so", RTLD_LAZY);
    void (*remove_block)(block_t*, int) = dlsym(libPtr,"remove_block");
#endif

    if(number > block->max_size)
        return;

    for(int i = 0; i < number; i++)
        remove_block(block, i);
}

void alternateReplace(block_t* block, int number){
#ifdef DLL
    void *libPtr = dlopen("./liblibrary.so", RTLD_LAZY);
    void (*set_block)(block_t*, const char*, int) = dlsym(libPtr,"set_block");
    void (*remove_block)(block_t*, int) = dlsym(libPtr,"remove_block");
#endif
    if(number > block->max_size)
        return;
    for(int i = 0; i < number; i++){
        remove_block(block, i);
        set_block(block, generate_string(block->block_size), i);
    }
}

void sequenceReplace(block_t* block, int number){
    removeBlocks(block, number);
    addBlocks(block, number);
}

struct timeval calcTime(struct timeval t1, struct timeval t2){
    struct timeval newTime;
    timersub(&t2, &t1, &newTime);
    return newTime;
}

//tests
void alloc_test(int size, int block_size, int mode){
#ifdef DLL
    void *libPtr = dlopen("./liblibrary.so", RTLD_LAZY);
    block_t* (*set_array)(int, int, int) = dlsym(libPtr,"set_array");
    void (*remove_array)(block_t*) = dlsym(libPtr,"remove_array");
#endif
    block_t* block;
    struct timeval t1, t2, user_start, user_end, system_start, system_end;
    struct rusage usage;

    getrusage(RUSAGE_SELF, &usage);
    system_start = usage.ru_stime;
    user_start = usage.ru_utime;
    gettimeofday(&t1, 0);

    for(int i = 0; i < 10000; i++){
        block = set_array(size, block_size, mode);
        remove_array(block);
    }

    gettimeofday(&t2, 0);
    getrusage(RUSAGE_SELF, &usage);
    system_end = usage.ru_stime;
    user_end = usage.ru_utime;

    printf("\nAlloc test: \n------------------------------\n");
    printf("Real time: %ld.%lds\n", calcTime(t1,t2).tv_sec, calcTime(t1,t2).tv_usec);
    printf("User time: %ld.%lds\n", calcTime(user_start, user_end).tv_sec, calcTime(user_start, user_end).tv_usec);
    printf("System time: %ld.%lds\n", calcTime(system_start, system_end).tv_sec, calcTime(system_start, system_end).tv_usec);
}

void search_test(int size, int block_size, int mode, const char* to_find){
#ifdef DLL
    void *libPtr = dlopen("./liblibrary.so", RTLD_LAZY);

    block_t* (*set_array)(int, int, int) = dlsym(libPtr,"set_array");
    void (*remove_array)(block_t*) = dlsym(libPtr,"remove_array");
    int (*find_block)(block_t*, const char*) = dlsym(libPtr,"find_block");
#endif
    block_t* block;
    int index;
    struct timeval t1, t2, user_start, user_end, system_start, system_end;
    struct rusage usage;

    getrusage(RUSAGE_SELF, &usage);
    system_start = usage.ru_stime;
    user_start = usage.ru_utime;
    gettimeofday(&t1, 0);

    block = set_array(size, block_size, mode);
    generateBlocks(block);

    for(int i = 0; i < 10000; i++)
        index = find_block(block, to_find);

    gettimeofday(&t2, 0);
    getrusage(RUSAGE_SELF, &usage);
    system_end = usage.ru_stime;
    user_end = usage.ru_utime;

    printf("\nSearch test: \n------------------------------\n");
    printf("Real time: %ld.%lds\n", calcTime(t1,t2).tv_sec, calcTime(t1,t2).tv_usec);
    printf("User time: %ld.%lds\n", calcTime(user_start, user_end).tv_sec, calcTime(user_start, user_end).tv_usec);
    printf("System time: %ld.%lds\n", calcTime(system_start, system_end).tv_sec, calcTime(system_start, system_end).tv_usec);

    remove_array(block);
}

void sequence_replace_test(int size, int block_size, int mode, int blocks_number){
#ifdef DLL
    void *libPtr = dlopen("./liblibrary.so", RTLD_LAZY);

    void (*remove_array)(block_t*) = dlsym(libPtr,"remove_array");
    block_t* (*set_array)(int, int, int) = dlsym(libPtr,"set_array");
#endif
    block_t* block;
    struct timeval t1, t2, user_start, user_end, system_start, system_end;
    struct rusage usage;

    getrusage(RUSAGE_SELF, &usage);
    system_start = usage.ru_stime;
    user_start = usage.ru_utime;
    gettimeofday(&t1, 0);

    block = set_array(size, block_size, mode);
    generateBlocks(block);

    for(int i = 0; i < 10000; i++)
        sequenceReplace(block, blocks_number);

    gettimeofday(&t2, 0);
    getrusage(RUSAGE_SELF, &usage);
    system_end = usage.ru_stime;
    user_end = usage.ru_utime;

    printf("\nSequence replace test: \n------------------------------\n");
    printf("Real time: %ld.%lds\n", calcTime(t1,t2).tv_sec, calcTime(t1,t2).tv_usec);
    printf("User time: %ld.%lds\n", calcTime(user_start, user_end).tv_sec, calcTime(user_start, user_end).tv_usec);
    printf("System time: %ld.%lds\n", calcTime(system_start, system_end).tv_sec, calcTime(system_start, system_end).tv_usec);

    remove_array(block);
}

void alternate_replace_test(int size, int block_size, int mode, int blocks_number){
#ifdef DLL
    void *libPtr = dlopen("./liblibrary.so", RTLD_LAZY);

    void (*remove_array)(block_t*) = dlsym(libPtr,"remove_array");
    block_t* (*set_array)(int, int, int) = dlsym(libPtr,"set_array");
#endif
    block_t* block;
    struct timeval t1, t2, user_start, user_end, system_start, system_end;
    struct rusage usage;

    getrusage(RUSAGE_SELF, &usage);
    system_start = usage.ru_stime;
    user_start = usage.ru_utime;
    gettimeofday(&t1, 0);

    block = set_array(size, block_size, mode);
    generateBlocks(block);

    for(int i = 0; i < 10000; i++)
        alternateReplace(block, blocks_number);

    gettimeofday(&t2, 0);
    getrusage(RUSAGE_SELF, &usage);
    system_end = usage.ru_stime;
    user_end = usage.ru_utime;

    printf("\nAlternate replace test: \n------------------------------\n");
    printf("Real time: %ld.%lds\n", calcTime(t1,t2).tv_sec, calcTime(t1,t2).tv_usec);
    printf("User time: %ld.%lds\n", calcTime(user_start, user_end).tv_sec, calcTime(user_start, user_end).tv_usec);
    printf("System time: %ld.%lds\n", calcTime(system_start, system_end).tv_sec, calcTime(system_start, system_end).tv_usec);

    remove_array(block);
}

int main(int argc, char* argv[]) {
#ifdef DLL
    void *libPtr = dlopen("./liblibrary.so", RTLD_LAZY);
    if(!libPtr){
        printf("%s\n", "Library cannot be opened!");
        return 1;
    }
#endif

    srand(time(NULL));

    //parameters
    int
            size = MAX_SIZE,
            block_size = BLOCK_SIZE,
            mode = 1,
            create = 0,
            blocks_to_add = 100,
            blocks_to_replace = 100;
    const char* to_find = "test";

    int next_option;

    const char* const short_options = "hs:b:m:cf:a:r:";

    const struct option long_options[] = {
            {"help",                0,  NULL,   'h'},
            {"size",                1,  NULL,   's'},
            {"block_size",          1,  NULL,   'b'},
            {"mode",                1,  NULL,   'm'},
            {"create",              0,  NULL,   'c'},
            {"find_element",        1,  NULL,   'f'},
            {"add",                 1,  NULL,   'a'},
            {"remove_and_add",      1,  NULL,   'r'},
            {NULL,                  0,  NULL,     0}
    };

    program_name = argv[0];

    do{
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);

        switch(next_option){
            case 'h':
                print_usage(stdout, 0);
                break;
            case 's':
                size = strtol(optarg, NULL, 10);
                break;
            case 'b':
                block_size = strtol(optarg, NULL, 10);
                break;
            case 'm':
                mode = strtol(optarg, NULL, 10);
                break;
            case 'c':
                create = 1;
                break;
            case 'f':
                to_find = optarg;
                break;
            case 'a':
                blocks_to_add = strtol(optarg, NULL, 10);
                break;
            case 'r':
                blocks_to_replace = strtol(optarg, NULL, 10);
                break;
            case '?':
                print_usage(stderr, 1);
            case -1:
                break;
            default:
                abort();
        }
    }while(next_option != -1);

    if(create) {
        printf("SIZE:    %d    BLOCK_SIZE:    %d    MODE:    %s\n", size, block_size, mode ? "DYNAMIC" : "STATIC");
        alloc_test(size, block_size, mode);
        search_test(size, block_size, mode, to_find);
        sequence_replace_test(size, block_size, mode, blocks_to_replace);
        alternate_replace_test(size, block_size, mode, blocks_to_replace);
    }

#ifdef DLL
    dlclose(libPtr);
#endif

    return 0;
}




