#include <stdio.h>
#include <malloc.h>
#include <zconf.h>
#include <memory.h>
#include <sys/wait.h>
#include <stdlib.h>
#define MAX_ARG 10
#define ARG_LENGTH 256

int main(int argc, char* argv[]){
    FILE* file = fopen(argv[1], "r");
    if(!file) exit(1);

    int status, argNumber = 0;
    char* arguments[MAX_ARG];
    char buffer[ARG_LENGTH];
    while(fgets(buffer, ARG_LENGTH, file)){
        argNumber = 0;
        char* rest = buffer;
        while((arguments[argNumber++] = strtok_r(NULL, " \n", &rest)));
        if(!vfork()){
            printf("Executing process with pid: %d\n", getpid());
            if(execvp(arguments[0], arguments) < 0) exit(1);
        }
        wait(&status);
    }
    printf("\n");
    return 0;
}