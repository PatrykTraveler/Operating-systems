#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bits/signum.h>
#include <signal.h>
#include <wait.h>

void child_process();
void parent_process();
void parent_handler(int, siginfo_t*, void*);
void child_handler(int, siginfo_t*, void*);

static int L;
static int type;
static pid_t child_pid;
static int sigcounter = 0;
static int childcounter = 0;
static int parentcounter = 0;

static void EXIT_ON_ERROR(int code, const char* msg){
    printf("%s", msg);
    exit(code);
}

int main(int argc, char* argv[]) {
    if(argc < 3)
        EXIT_ON_ERROR(EXIT_FAILURE, "Insufficient number of parameters.\n");

    L = (int)strtol(argv[1], '\0', 10);
    type = (int)strtol(argv[2], '\0',  10);

    child_pid = fork();
    if(child_pid == 0)
        child_process();
    else if(child_pid > 0)
        parent_process();
    else
        EXIT_ON_ERROR(EXIT_FAILURE, "An error has occured while forking\n");

    return 0;
}

void parent_handler(int signum, siginfo_t *info, void *context){
    if(signum == SIGINT){
        type == 3 ? kill(child_pid, SIGRTMAX) : kill(child_pid, SIGUSR2);
        printf("Signals sent to child: %d\n", childcounter);
        printf("Signals received from child: %d\n", parentcounter);
        exit(0);
    }
    if(info->si_pid != child_pid) return;
    if(signum == SIGUSR1 || signum == SIGRTMIN){
        parentcounter++;
        printf("Parent has received %s from child\n", signum == SIGUSR1 ? "SIGUSR1" : "SIGRTMIN");
    }
}

void child_handler(int signum, siginfo_t *info, void *context){
    if(info->si_pid != getppid()) return;

    if(signum == SIGUSR1 || signum == SIGRTMIN) {
        childcounter++;
        kill(getppid(), signum);
        printf("Child has received %s and sent back to parent\n", signum == SIGUSR1 ? "SIGUSR1" : "SIGRTMIN");
    }
    if(signum == SIGUSR2 || signum == SIGRTMAX){
        childcounter++;
        printf("Child has received %s. Terminating...\n", signum == SIGUSR2 ? "SIGUSR2" : "SIGRTMAX");
        printf("Signals received by child %d\n", childcounter);
        exit(0);
    }
}

void child_process(){
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = child_handler;

    sigset_t signal_set;
    sigfillset(&signal_set);
    if(type == 1 || type == 2){
        if(sigaction(SIGUSR1, &act, NULL) == -1)
            EXIT_ON_ERROR(EXIT_FAILURE, "CHILD: Cannot catch SIGUSR1\n");
        if(sigaction(SIGUSR2, &act, NULL) == -1)
            EXIT_ON_ERROR(EXIT_FAILURE, "CHILD: Cannot catch SIGUSR2\n");
        sigdelset(&signal_set, SIGUSR1);
        sigdelset(&signal_set, SIGUSR2);
    }
    else if(type == 3){
        if(sigaction(SIGRTMIN, &act, NULL) == -1)
            EXIT_ON_ERROR(EXIT_FAILURE, "CHILD: Cannot catch SIGRTMIN\n");
        if(sigaction(SIGRTMAX, &act, NULL) == -1)
            EXIT_ON_ERROR(EXIT_FAILURE, "CHILD: Cannot catch SIGRTMAX\n");
        sigdelset(&signal_set, SIGRTMIN);
        sigdelset(&signal_set, SIGRTMAX);
    }
    sigprocmask(SIG_SETMASK, &signal_set, NULL);
    while(1) pause();
}

void parent_process(){
    sleep(1);
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = parent_handler;

    if(sigaction(SIGINT, &act, NULL) == -1) EXIT_ON_ERROR(EXIT_FAILURE, "Cannot catch SIGINT\n");
    if(type == 1 || type == 2)
        if(sigaction(SIGUSR1, &act, NULL) == -1)
            EXIT_ON_ERROR(EXIT_FAILURE, "PARENT: Cannot catch SIGUSR1\n");
    if(type == 3)
        if(sigaction(SIGRTMIN, &act, NULL) == -1)
            EXIT_ON_ERROR(EXIT_FAILURE, "PARENT: Cannot catch SIGRTMIN\n");

    if(type == 1 || type == 2){
        sigset_t signal_set;
        sigfillset(&signal_set);
        sigdelset(&signal_set, SIGUSR1);
        sigdelset(&signal_set, SIGINT);

        while(sigcounter++ < L){
            printf("Sending %d'th SIGUSR1 signal to child.\n", sigcounter);
            kill(child_pid, SIGUSR1);
            if(type == 2)
                sigsuspend(&signal_set);
        }
        sigcounter++;
        printf("Sending SIGUSR2 to child\n");
        kill(child_pid, SIGUSR2);
    }else if(type == 3){
        while(sigcounter++ < L){
            printf("Sending SIGRTMIN to child\n");
            kill(child_pid, SIGRTMIN);
        }
        sigcounter++;
        printf("Sending SIGRTMAX to child.\n");
        kill(child_pid, SIGRTMAX);
    }

    int status = 0;
    waitpid(child_pid, &status, 0);
    printf("Signals sent to child: %d\n", sigcounter);
    printf("Signals received from child: %d\n", parentcounter);
    printf("Parent process finished with status: %d\n", status);
    exit(EXIT_SUCCESS);
}

