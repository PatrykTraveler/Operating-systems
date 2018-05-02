#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include <sys/msg.h>
#include <bits/signum.h>
#include <signal.h>
#include <unistd.h>
#include "utilities.h"
#include <errno.h>

#define EXIT_ON_ERROR(format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(EXIT_FAILURE);}
#define STR_EQUAL(x, y) strcmp(x, y) == 0
#define LINE_MAX 128

int clientID = -1;
int queueID = -1;
int input_mode;

void setClient(key_t);
void fmirror(msg_t*);
void fcalc(msg_t*);
void ftime(msg_t*);
void fend(msg_t*);

void remove_queue(){
    if (clientID > -1)
    {
        if (msgctl(clientID, IPC_RMID, NULL) < 0) printf("Couldn't remove client queue atexit.\n");
        printf("CLIENT %d: Ending program after having removed queue.\n",getpid());
    }
    if (queueID > -1)
    {
        msg_t msg;
        msg.mtype = STOP;
        msg.request_pid = getpid();
        if (msgsnd(queueID, &msg, MSG_SIZE, 0) < 0) printf("Couldn't send stop msg to server in process %d. Prolly server already stopped working.\n", getpid());
        else printf("CLIENT %d: Sent STOP msg to server. Should remove my queue from array.\n", getpid());
    }
}

void sigint_handler(int signum){
	msgctl(clientID, IPC_RMID, NULL);
	exit(0);
}

int main(int argc, char *argv[]) {
    signal (SIGINT, sigint_handler);
    input_mode = argc == 2 ? 0 : 1;
    if (atexit(remove_queue) < 0) EXIT_ON_ERROR("Couldn't register atexit function.\n");

    char* path = getenv("HOME");
    if (path == NULL) EXIT_ON_ERROR("Couldn't get HOME envvar.\n");

    key_t main_key = ftok(path, PROJECT_ID);
    if(main_key == -1) EXIT_ON_ERROR("Failed to generate main key\n");

    queueID = msgget(main_key, 0);
    if(queueID == -1) EXIT_ON_ERROR("Failed to access server's queue\n");

    key_t client_key = ftok(path, getpid());
    if(client_key == -1) EXIT_ON_ERROR("Failed to generate client key\n");

    clientID = msgget(client_key, IPC_CREAT | 0666);
    if(clientID == -1) EXIT_ON_ERROR("Failed to access server's queue\n");
    setClient(client_key);

    char command[20];
    msg_t msg;
    while(input_mode){
        msg.request_pid = getpid();
        printf("Enter your command: ");
        if(fgets(command, 20, stdin) == NULL){
            printf("Failed to read command\n");
            continue;
        }

        int n = (int)strlen(command);
        if(command[n-1] == '\n') command[n-1] = '\0';

        if(STR_EQUAL(command, "mirror")) fmirror(&msg);
        else if(STR_EQUAL(command, "calc")) fcalc(&msg);
        else if(STR_EQUAL(command, "time")) ftime(&msg);
        else if(STR_EQUAL(command, "end")) fend(&msg);
        else if(STR_EQUAL(command, "q")) exit(0);
        else printf("Invalid command!\n");
    }

    if(!input_mode){
        FILE *file = fopen(argv[1], "r");
        if(file == NULL) EXIT_ON_ERROR("Cannot open file!\n");
        char line[LINE_MAX];
        while(fgets(line, LINE_MAX, file)){
            if(line[0] == '\n') continue;
            char* rest = line;
            char *arg = strtok_r(NULL, " \n\t", &rest);

            msg.request_pid = getpid();
            if(STR_EQUAL(arg, "mirror")) {
                sprintf(msg.buffer, "%s", rest);
                fmirror(&msg);
            }
            else if(STR_EQUAL(arg, "calc")){
                sprintf(msg.buffer, "%s", rest);
                fcalc(&msg);
            }
            else if(STR_EQUAL(arg, "time")){
                ftime(&msg);
            }
            else if(STR_EQUAL(arg, "end")){
                fend(&msg);
            }
            else{
                printf("Invalid command!\n");
            }
        }
    }

    return 0;
}

void setClient(key_t client_key){
    msg_t msg;
    msg.request_pid = getpid();
    msg.mtype = BEGIN;
    sprintf(msg.buffer, "%d", client_key);

    if(msgsnd(queueID, &msg, MSG_SIZE, 0) < 0) EXIT_ON_ERROR("Couldn't send start request to server in pid: %d\n", getpid());
    if (msgrcv(clientID, &msg, MSG_SIZE, 0 ,0) < 0) EXIT_ON_ERROR("Couldn't receive start permission request to server in pid: %d\n", getpid());
    int ID; 
    if((ID = (int)strtol(msg.buffer, NULL, 10)) == -1) EXIT_ON_ERROR("No place for client of pid: %d\n", getpid());

    printf("Registered client of pid: %d has ID : %d\n", getpid(), ID);
}

void fmirror(msg_t *msg){
    msg->mtype = MIRROR;
    if(input_mode) {
        printf("Enter string to mirror: ");
        if (fgets(msg->buffer, MSG_SIZE, stdin) == NULL) printf("String is too long\n");
    }
    if(msgsnd(queueID, msg, MSG_SIZE, 0) < 0) EXIT_ON_ERROR("Cannot send MIRROR request to server\n");
    if(msgrcv(clientID, msg, MSG_SIZE, 0, 0) < 0) EXIT_ON_ERROR("Cannot receive MIRROR request from server ERRNO: %d\n", errno);
    printf("%s\n", msg->buffer);
}

void fcalc(msg_t *msg){
    msg->mtype = CALC;
    if(input_mode) {
        printf("Enter expression to calculate: ");
        if (fgets(msg->buffer, MSG_SIZE, stdin) == NULL) printf("Expression is too long\n");
    }
    if(msgsnd(queueID, msg, MSG_SIZE, 0) < 0) EXIT_ON_ERROR("Cannot send CALC request to server\n");
    if(msgrcv(clientID, msg, MSG_SIZE, 0, 0) < 0) EXIT_ON_ERROR("Cannot receive CALC request from server\n");
    printf("%s\n", msg->buffer);
}

void ftime(msg_t *msg){
    msg->mtype = TIME;
    if(msgsnd(queueID, msg, MSG_SIZE, 0) < 0) EXIT_ON_ERROR("Cannot send TIME request to server\n");
    if(msgrcv(clientID, msg, MSG_SIZE, 0, 0) < 0) EXIT_ON_ERROR("Cannot receive TIME request from server\n");
    printf("%s\n", msg->buffer);
}

void fend(msg_t *msg){
    msg->mtype = END;
    if(msgsnd(queueID, msg, MSG_SIZE, 0) < 0) EXIT_ON_ERROR("Cannot send END request to server\n");
}
