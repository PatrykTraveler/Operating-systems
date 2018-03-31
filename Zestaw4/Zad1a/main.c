#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <zconf.h>
#include <stdlib.h>

int stop_f = 0;
void sigHandler(int signal){
    switch(signal) {
        case SIGTSTP:
        if ((stop_f = stop_f ? 0 : 1))
            printf("\n Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
            break;
        case SIGINT:
            printf("\n Odebrano sygnał SIGINT\n");
            exit(0);
        default:
            break;
    }
}

void timestamp(){
    time_t t = time(0);
    time(&t);
    printf("The time is %s", ctime(&t));
}

int main() {
    struct sigaction sigact;

    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = sigHandler;

    sigaction(SIGINT, &sigact, NULL);
    signal(SIGTSTP, sigHandler);
    while(1) {
        timestamp();
        sleep(1);
        if(stop_f) pause();
    }
}