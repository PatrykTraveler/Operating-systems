#include <stdio.h>
#include <malloc.h>
#include <zconf.h>
#include <memory.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_ARG 10
#define ARG_LENGTH 256

long mbToBytes(long val){
    return val*1024*1024;
}

struct timeval calcTime(struct timeval t1, struct timeval t2){
    struct timeval newTime;
    timersub(&t2, &t1, &newTime);
    return newTime;
}

void getUsage(struct rusage cRusage[2]){
    struct timeval userTime, systemTime;
    systemTime = calcTime(cRusage[0].ru_stime, cRusage[1].ru_stime);
    userTime = calcTime(cRusage[0].ru_utime, cRusage[1].ru_utime);
    long int memUsage = cRusage[1].ru_maxrss;

    printf("\nUser time: %ld.%lds\n", userTime.tv_sec, userTime.tv_usec);
    printf("System time: %ld.%lds\n", systemTime.tv_sec, systemTime.tv_usec);
    printf("Max memory: %ld\n", memUsage);
}

void initLimit(long cLimit, long mLimit){
    struct rlimit cpuLimit = {(rlim_t)cLimit, (rlim_t)cLimit}, memLimit = {(rlim_t)mLimit, (rlim_t)mLimit};

    if(setrlimit(RLIMIT_CPU, &cpuLimit) == -1) printf("Unable to make cpu limit!\n");
    if(setrlimit(RLIMIT_DATA, &memLimit) == -1) printf("Unable to make memory limit!\n");
    if(setrlimit(RLIMIT_STACK, &memLimit) == -1) printf("Unable to make memory limit!\n");

    printf("\nCPU LIMIT: %ld\t MEM LIMIT: %ld\n", cLimit, mLimit);
}

int main(int argc, char* argv[]){
    FILE* file = fopen(argv[1], "r");
    if(!file){
        printf("File not found."); exit(1);
    }

    int status, argNumber = 0;
    char* arguments[MAX_ARG];
    char buffer[ARG_LENGTH];
    struct rusage childrenRusage[2];

    while(fgets(buffer, ARG_LENGTH, file)){
        argNumber = 0;
        char* rest = buffer;
        while((arguments[argNumber++] = strtok_r(NULL, " \n", &rest)));

        getrusage(RUSAGE_CHILDREN, &childrenRusage[0]);

        pid_t process_id = vfork();
        if(!process_id){
            printf("\nExecuting process with pid: %d\n", getpid());
            initLimit(strtol(argv[2], NULL, 10), mbToBytes(strtol(argv[3], NULL, 10)));
            if(execvp(arguments[0], arguments) < 0) exit(1);
        }
        wait(&status);

        getrusage(RUSAGE_CHILDREN, &childrenRusage[1]);
        getUsage(childrenRusage);
    }
    printf("\n");
    return 0;
}