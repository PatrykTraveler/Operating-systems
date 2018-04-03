#define  _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static pid_t internal_child_pid = 0;
int received = 0;

void signal_handler(int signum){
    if(signum == SIGTSTP) {
        received = received ? 0 : 1;
        printf("%s by signal %d\n", received ? "Stopped" : "Resumed", signum);
        if (!internal_child_pid)
            kill(internal_child_pid, SIGKILL);
    }
}


int main(int argc, char *argv[])
{
    int status;
    pid_t pid;
    struct sigaction sigact;

    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = signal_handler;

    sigaction(SIGCHLD, &sigact, NULL);
    sigaction(SIGTSTP, &sigact, NULL);
    while(1) {
        if(!received) {
            pid = fork();
            internal_child_pid = pid;
            if (!pid) {
                if (execvp(argv[1], argv + 1) < 0)
                    exit(EXIT_FAILURE);
            }
            waitpid(pid, &status, NULL);
        }
    }
}