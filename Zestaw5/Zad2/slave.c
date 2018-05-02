#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <fcntl.h>

#define EXIT_ON_ERROR(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define MAX_BUFF 1024

int main(int argc, char *argv[]) {
    FILE *date;
    srand((unsigned int)getpid()*getppid());

    if(argc < 3) EXIT_ON_ERROR(EXIT_FAILURE, "Wrong number of parameters.\n");
    int N = (int)strtol(argv[2], '\0', 10);
    int fifo = open(argv[1], O_WRONLY);
    if(fifo < 0) EXIT_ON_ERROR(EXIT_FAILURE, "Master: failed to open fifo\n");

    printf("Slave %d: my PID is: %d", N, getpid());
    char buffer[2][MAX_BUFF];
    for(int i = 0; i < N; i++){
        date = popen("date", "r");
        fgets(buffer[0], MAX_BUFF, date);
        sprintf(buffer[1], "PID: %d DATE: %s", getpid(), buffer[0]);
        write(fifo, buffer[1], strlen(buffer[1]));
        fclose(date);
        sleep(rand() % 5);
    }

    close(fifo);

    return 0;
}
