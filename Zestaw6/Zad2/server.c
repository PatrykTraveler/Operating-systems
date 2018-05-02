#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
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
        for(int i = 0; i < clients_counter; i++){
            mq_close(clients_arr[i].qid);
            kill(clients_arr[i].pid, SIGINT);
        }
        if(mq_close(queueID) < 0) printf("SERVER: Failed to close main queue atexit\n");
        if(mq_unlink(qPath) < 0) printf("SERVER: Failed to unlink main queue\n");
        else printf("SERVER: Ending program after having removed main queue.\n");
    }
}

void sigint_handler(int signum){
    if(signum == SIGINT) exit(1);
}

int main(int argc, char* argv[]){
    if (atexit(remove_queue) < 0) EXIT_ON_ERROR("Couldn't register atexit function.\n");
    if(signal(SIGINT, sigint_handler) == SIG_ERR) EXIT_ON_ERROR("Registering INT failed!");

    struct mq_attr posixAttr;
    posixAttr.mq_maxmsg = MSG_LIMIT; 
    posixAttr.mq_msgsize = MSG_SIZE;

    queueID = mq_open(qPath, O_RDONLY | O_CREAT | O_EXCL, 0666, &posixAttr);
    if(queueID == -1) EXIT_ON_ERROR("Failed to create public queue!\n");

    struct mq_attr info;

    msg_t msg;
    while(1){
        if(!is_running) {
            if (mq_getattr(queueID, &info) != 0) EXIT_ON_ERROR("Failed to receive queue status\n");
            if(info.mq_curmsgs == 0){
                printf("SERVER: Ending main queue.\n");
                exit(0);
            }
        }
        if (mq_receive(queueID, (char*)&msg, MSG_SIZE, NULL) < 0) EXIT_ON_ERROR("Failed to receive message\n");
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
    char client_path[15];
    int client_pid = msg->request_pid;
    sprintf(client_path, "/%d", client_pid);
    int client_queue = mq_open(client_path, O_WRONLY);
    if(client_queue == -1) EXIT_ON_ERROR("Failed to open client's queue\n");
    msg->request_pid = getpid();
    msg->mtype = BEGIN;

    if(clients_counter >= MAX_CLIENTS){
        printf("Maximum number of clients has been exceeded\n");
        sprintf(msg->buffer, "%d", -1);
        if(mq_send(client_queue, (char*)msg, MSG_SIZE, 1) == -1) EXIT_ON_ERROR("Failed to send info to client\n");
        if(mq_close(client_queue) < 0) EXIT_ON_ERROR("Failed to close client's queue\n");
    }
    else{
        clients_arr[clients_counter].pid = client_pid;
        clients_arr[clients_counter].qid = client_queue;
        sprintf(msg->buffer, "%d", clients_counter);
        clients_counter++;
        if(mq_send(client_queue, (char*)msg, MSG_SIZE, 1) == -1){ 
            EXIT_ON_ERROR("Failed to send access to queue to client\n");
        }
        else{ 
            printf("Registered process %d with client ID %d\n", msg->request_pid, clients_counter - 1);
        }
    }
}

void fmirror(msg_t *msg){
    int client_q = prepare_msg(msg);
    if(client_q == -1) EXIT_ON_ERROR("Cannot access client's queue\n");

    int N = strlen(msg->buffer);
    char *result = calloc(sizeof(char), N);
    for(int i = 0; i < N; i++) result[i] = msg->buffer[N-i-1];

    sprintf(msg->buffer, "%s", result);
    if(mq_send(client_q, (char*)msg, MSG_SIZE, 2) == -1) EXIT_ON_ERROR("Failed to send MIRROR result to queue to client\n");
}

void fcalc(msg_t *msg){
    int client_q = prepare_msg(msg);
    if(client_q == -1) EXIT_ON_ERROR("Cannot access client's queue\n");
    char args[MSG_SIZE];
    sprintf(args, "echo '%s' | bc", msg->buffer);
    FILE* calc = popen(args, "r");
    fgets(msg->buffer, MSG_SIZE, calc);
    pclose(calc);
    if(mq_send(client_q, (char*)msg, MSG_SIZE, 2) == -1) EXIT_ON_ERROR("Failed to send CALC result to queue to client\n");
}

void ftime(msg_t *msg){
    int client_q = prepare_msg(msg);
    if(client_q == -1) EXIT_ON_ERROR("Cannot access client's queue\n");
    char buffer[MSG_SIZE];
    time_t current_time = time(NULL);
    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", localtime(&current_time));
    sprintf(msg->buffer, "%s", buffer);
    if(mq_send(client_q, (char*)msg, MSG_SIZE, 2) == -1) EXIT_ON_ERROR("Failed to send TIME result to queue to client\n");
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
    if(--i == clients_counter){
        printf("Client not found\n");
        return;
    }
    if(mq_close(clients_arr[i].qid) == -1) EXIT_ON_ERROR("Failed to close client's queue\n");
    for(; i <= clients_counter; i++){
        clients_arr[i].pid = clients_arr[i+1].pid;
        clients_arr[i].qid = clients_arr[i+1].qid;
    }
    clients_counter--;
    printf("SERVER: Client has been successfully removed form clients array\n");
}