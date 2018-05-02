#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <bits/siginfo.h>
#include <wait.h>
#include <errno.h>
#include <memory.h>

void sigint_handler(int signum, siginfo_t *info, void *context);
void sigusr_handler(int signum, siginfo_t *info, void *context);
void sigchld_handler(int signum, siginfo_t *info, void *context);
void rest_handler(int signum, siginfo_t *info, void *context);

static int N, K, n, request_counter;
static pid_t *children_pids;
static pid_t *waiting_pids;

int main(int argc, char* argv[]){
    if(argc < 3) {
        printf("Wrong number of parameters.");
        exit(EXIT_FAILURE);
    }
    N = (int)strtol(argv[1], '\0',  10);
    K = (int)strtol(argv[2], '\0', 10);
    n = 0;
    request_counter = 0;
    children_pids = calloc(sizeof(pid_t), N);
    waiting_pids = calloc(sizeof(pid_t), N);

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    act.sa_sigaction = sigint_handler;
    if(sigaction(SIGINT, &act, NULL) == -1) exit(EXIT_FAILURE);
    act.sa_sigaction = sigusr_handler;
    if(sigaction(SIGUSR1, &act, NULL) == -1) exit(EXIT_FAILURE);
    act.sa_sigaction = sigchld_handler;
    if(sigaction(SIGCHLD, &act, NULL) == -1) exit(EXIT_FAILURE);

    for(int i = SIGRTMIN; i <= SIGRTMAX; i++){
        act.sa_sigaction = rest_handler;
        if(sigaction(i, &act, NULL) == -1) exit(EXIT_FAILURE);
    }

    for(int i = 0; i < N; i++){
        usleep(26210);
        pid_t child_pid = fork();
        if(!child_pid){
            execl("./child", "./child", NULL);
            exit(EXIT_FAILURE);
        }
        else
            children_pids[n++] = child_pid;
    }

    while(waitpid(-1, NULL, 0));
    return 0;
}

void sigint_handler(int signum, siginfo_t *info, void *context){
    int finished = 0;

    printf("Received SIGINT. Killing all the processes...\n");

    for(int i = 0; i < N; i++){
        if(children_pids[i] > 0){
            if(kill(children_pids[i], SIGKILL) == -1)
                finished++;
            waitpid(children_pids[i], NULL, 0);
        }
    }

    printf("All the processes have been killed. %d has finished before termination.\n", finished);

    exit(EXIT_SUCCESS);
}

void sigusr_handler(int signum, siginfo_t *info, void *context){
    printf("Received [%s] from process with pid: %d\n",strsignal(signum), info->si_pid);

    if(request_counter >= K){
        kill(info->si_pid, SIGUSR1);
        waitpid(info->si_pid, NULL, 0);
    }
    else{
        waiting_pids[request_counter++] = info->si_pid;
        if(request_counter >= K){
            for(int i = 0; i < request_counter; i++){
                if(waiting_pids[i] > 0){
                    kill(waiting_pids[i], SIGUSR1);
                    waitpid(waiting_pids[i], NULL, 0);
                }
            }
        }
    }
}

void sigchld_handler(int signum, siginfo_t *info, void *context){
    printf("Process with PID %d has returned with value %d.\n", info->si_pid, info->si_status);
    if((--n) == 0){
        printf("There are no any children left. Terminating parent process...\n");
        exit(EXIT_SUCCESS);
    }
}

void rest_handler(int signum, siginfo_t *info, void *context){
    printf("Received SIGRTMIN+%d from process with pid: %d\n", signum-SIGRTMIN, info->si_pid);
}