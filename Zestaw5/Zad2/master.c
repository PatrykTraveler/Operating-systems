#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

#define EXIT_ON_ERROR(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define MAX_PATH 60;
#define MAX_BUFF 1024

int main(int argc, char *argv[]) {
    if(argc < 2) EXIT_ON_ERROR(EXIT_FAILURE, "Wrong number of parameters.\n");
    if(mkfifo(argv[1], S_IRUSR | S_IWUSR) == -1) EXIT_ON_ERROR(EXIT_FAILURE, "Master: failed to create FIFO\n");

    FILE *fifo = fopen(argv[1], "r");
    if(fifo == NULL) EXIT_ON_ERROR(EXIT_FAILURE, "Master: failed to open fifo\n");

    char buff[MAX_BUFF];
    while(fgets(buff, MAX_BUFF, fifo)) write(STDOUT_FILENO, buff, strlen(buff));
    fclose(fifo);

    return 0;
}