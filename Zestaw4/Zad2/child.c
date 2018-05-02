#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

void sigusr_handler(int signum){
    kill(getppid(), rand() % (SIGRTMAX - SIGRTMIN) + SIGRTMIN);
}

int main(){
    int sleep_t;

    srand(getpid());
    signal(SIGUSR1, sigusr_handler);

    sigset_t signal_set;
    sigfillset(&signal_set);
    sigdelset(&signal_set, SIGUSR1);

    sleep_t = rand() % 11;

    sleep(sleep_t);
    kill(getppid(), SIGUSR1);
    sigsuspend(&signal_set);

    return sleep_t;
}
