#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include "utilities.h"

#define EXIT_ON_ERROR(format, ...){fprintf(stderr, format, ##__VA_ARGS__); exit(EXIT_FAILURE);}

typedef struct client_info{
    int pid, qid;
}client_info;

int is_running = 1;
int queueID = -1;
int clientID = -1;

static client_info clients_arr[MAX_CLIENTS];
static int clients_counter = 0;

int find_client_queue(pid_t);
int prepare_msg(msg_t*);
void fbegin(msg_t*);
void fcalc(msg_t*);
void fmirror(msg_t*);
void ftime(msg_t*);
void remove_client(msg_t*);
void requestHandler(msg_t*);

void remove_queue(){
    if(queueID > -1){
        if(msgctl(queueID, IPC_RMID, NULL) < 0) EXIT_ON_ERROR("Failed to remove main queue\n");
        for (int i = 0; i <= clients_counter; i++) kill(clients_arr[i].pid, SIGINT);
        printf("SERVER: Ending program after having removed queue.\n");
    }
}

void sigint_handler(int signum){
    msgctl(queueID, IPC_RMID, NULL);
    exit(0);
}

int main(int argc, char* argv[]){
    signal(SIGINT, sigint_handler);

    if (atexit(remove_queue) < 0) EXIT_ON_ERROR("Couldn't register atexit function.\n");
    char* path = getenv("HOME");

    if (path == NULL) EXIT_ON_ERROR("Couldn't get HOME envvar.\n");
    key_t mainkey = ftok(path, PROJECT_ID);
    if(mainkey == -1) EXIT_ON_ERROR("Failed to generate key\n");
    if((queueID = msgget(mainkey, IPC_CREAT | 0666)) < 0)
        EXIT_ON_ERROR("Error Creating Message Queue\n")

    msg_t msg;
    struct msqid_ds queue_state;
    while(1){
        if(!is_running) {
            if (msgctl(queueID, IPC_STAT, &queue_state) < 0) EXIT_ON_ERROR("Failed to receive queue status\n");
            if(queue_state.msg_qnum == 0){
                printf("SERVER: Ending main queue.\n");
                break;
            }
        }
        if (msgrcv(queueID, &msg, MSG_SIZE, 0, 0) < 0) EXIT_ON_ERROR("Failed to receive message\n");
        requestHandler(&msg);
    }
    return 0;
}

void requestHandler(msg_t *msg){
    if(msg == NULL)
        return;
    switch(msg->mtype){
        case BEGIN:
            fbegin(msg);
            break;
        case MIRROR:
            fmirror(msg);
            break;
        case CALC:
            fcalc(msg);
            break;
        case TIME:
            ftime(msg);
            break;
        case END:
            is_running = 0;
            break;
        case STOP:
            remove_client(msg);
            break;
        default:
            break;
    }
}

void fbegin(msg_t *msg){
    key_t client_key = (key_t)strtol(msg->buffer, NULL, 10);    //get access to client's queue
    int client_q = msgget(client_key, 0);
    int client_pid = msg->request_pid;
    msg->request_pid = getpid();
    msg->mtype = BEGIN;

    if(clients_counter >= MAX_CLIENTS){
        printf("Maximum number of clients has been exceeded\n");
        sprintf(msg->buffer, "%d", -1);
    }
    else{
        clients_arr[clients_counter].pid = client_pid;
        clients_arr[clients_counter].qid = client_q;
        sprintf(msg->buffer, "%d", clients_counter);
        clients_counter++;
    }

    if(msgsnd(client_q, msg, MSG_SIZE, 0) < 0) EXIT_ON_ERROR("Cannot access client's queue\n");
    printf("Registered process %d with client ID %d\n", msg->request_pid, clients_counter - 1);
}

void fmirror(msg_t *msg){
    int client_q = prepare_msg(msg);
    if(client_q == -1) EXIT_ON_ERROR("Cannot access client's queue\n");

    int N = strlen(msg->buffer);
    char *result = calloc(sizeof(char), N);
    for(int i = 0; i < N; i++) result[i] = msg->buffer[N-i-1];

    sprintf(msg->buffer, "%s", result);
    if(msgsnd(client_q, msg, MSG_SIZE, 0) < 0) EXIT_ON_ERROR("Failed to perform MIRROR\n");
}

void fcalc(msg_t *msg){
    int client_q = prepare_msg(msg);
    if(client_q == -1) EXIT_ON_ERROR("Cannot access client's queue\n");
    char args[MSG_SIZE];
    sprintf(args, "echo '%s' | bc", msg->buffer);
    FILE* calc = popen(args, "r");
    fgets(msg->buffer, MSG_SIZE, calc);
    pclose(calc);
    if(msgsnd(client_q, msg, MSG_SIZE, 0) < 0) EXIT_ON_ERROR("Failed to perform CALC\n");
}

void ftime(msg_t *msg){
    int client_q = prepare_msg(msg);
    if(client_q == -1) EXIT_ON_ERROR("Cannot access client's queue\n");
    char buffer[MSG_SIZE];
    time_t current_time = time(NULL);
    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", localtime(&current_time));
    sprintf(msg->buffer, "%s", buffer);
    if(msgsnd(client_q, msg, MSG_SIZE, 0) < 0) EXIT_ON_ERROR("Failed to perform TIME\n");
}

int prepare_msg(msg_t *msg){
    int client_q = find_client_queue(msg->request_pid);
    msg->mtype = msg->request_pid;
    msg->request_pid = getpid();
    return client_q;
}

int find_client_queue(pid_t client_pid){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients_arr[i].pid == client_pid) return clients_arr[i].qid;
    }
    return -1;
}

void remove_client(msg_t *msg){
    int i = 0;
    while(clients_arr[i++].pid != msg->request_pid);
    i--;
    for(; i <= clients_counter; i++){
        clients_arr[i].pid = clients_arr[i+1].pid;
        clients_arr[i].qid = clients_arr[i+1].qid;
    }
    clients_counter--;
    printf("SERVER: Client has been successfully removed form clients array\n");
}