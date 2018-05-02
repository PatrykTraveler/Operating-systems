#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


#define ARGS_MAX 32
#define CMD_MAX 32
#define LINE_MAX 120
#define EXIT_ON_ERROR(code, format, ...) {fprintf (stderr, format, ##__VA_ARGS__); exit(code);}

int main(int argc, char *argv[]) {
    if(argc < 2)
    EXIT_ON_ERROR(EXIT_FAILURE, "You have to provide input file\n");

    char* filename = argv[1];
    FILE *file = fopen(filename, "r");
    if(file == NULL)
    EXIT_ON_ERROR(EXIT_FAILURE, "Cannot open the file\n");

    char line[ARGS_MAX];
    char* tmp_line;

    char *args[ARGS_MAX], *cmds[CMD_MAX];
    int cmd_counter = 0, args_counter = 0, line_counter = 0;
    volatile int i = 0;

    while(fgets(line, LINE_MAX, file)){
        line_counter++;
        tmp_line = strdup(line);
        cmd_counter = 0;
        while((cmds[cmd_counter++] = strsep(&tmp_line, "|")) != NULL);
        cmd_counter--;
        int (*fds)[2] = calloc(sizeof(int[2]), cmd_counter);
        for (i = 0; i < cmd_counter; i++) {
            char* buff = cmds[i];
            args_counter = 0;
            while((args[args_counter++] = strtok_r(NULL, " \t\n", &buff)) != NULL);
            if(i < cmd_counter -1)
                if (pipe(fds[i]))
                    printf("Couldn't pipe at %d\n", i);

            pid_t child = fork();
            if (child == -1) EXIT_ON_ERROR(EXIT_FAILURE, "Error while forking process %d\n", i);
            if (child == 0) {
                if(i > 0)
                    if(dup2(fds[i-1][0], STDIN_FILENO) < 0) EXIT_ON_ERROR(EXIT_FAILURE, "Failed to dup\n");

                if(i < cmd_counter - 1){
                    close(fds[i][0]);
                    if(dup2(fds[i][1], STDOUT_FILENO) < 0) EXIT_ON_ERROR(EXIT_FAILURE, "Failed to dup\n");
                }
                if (execvp(args[0], args) < 0) EXIT_ON_ERROR(EXIT_FAILURE, "Failed to execute %d process\n", i);
            }
            else{
                if(i < cmd_counter - 1)
                    close(fds[i][1]);
                if(i > 0)
                    close(fds[i-1][0]);
            }
        }
        while(wait(NULL)){
            if(errno == ECHILD) break;
        }
    }
    fclose(file);
}
