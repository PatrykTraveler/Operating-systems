#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>

#include "utilities.h"

#define EXIT_ON_ERROR(format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(EXIT_FAILURE);}
#define STR_EQUAL(x, y) strcmp(x, y) == 0
#define LINE_MAX 128

int clientID = -1;
int queueID = -1;
int input_mode;

static char path[30];

void setClient();
void fmirror(msg_t*);
void fcalc(msg_t*);
void ftime(msg_t*);
void fend(msg_t*);

void remove_queue(){
    if (queueID > -1 || clientID > -1)
    {
    	msg_t msg;
    	msg.mtype = STOP;
        msg.request_pid = getpid();
        if (mq_send(queueID, (char *)&msg, MSG_SIZE, 2) < 0) EXIT_ON_ERROR("Couldn't send close msg to server in process %d. Prolly server already stopped working.\n", getpid());
        if (mq_close(queueID)< 0) printf("CLIENT: Couldn't close main queue atexit.\n");
        if (mq_close(queueID) < 0) printf("CLIENT: Couldn't close client queue atexit.\n");
        if (mq_unlink(path) < 0) printf("Couldn't remove client queue atexit.\n");
        printf("CLIENT: Ending program after having removed queue.\n");
}
}

int main(int argc, char *argv[]) {
    input_mode = argc == 2 ? 0 : 1;
    if (atexit(remove_queue) < 0) EXIT_ON_ERROR("Couldn't register atexit function.\n");

    queueID = mq_open(qPath, O_WRONLY);
    if(queueID == -1) EXIT_ON_ERROR("Failed to open public queue\n");

    struct mq_attr posixAttr;
    posixAttr.mq_maxmsg = MSG_LIMIT;
    posixAttr.mq_msgsize = MSG_SIZE;

    sprintf(path, "/%d", getpid());
    clientID = mq_open(path, O_RDONLY | O_CREAT | O_EXCL, 0666, &posixAttr);
    if(clientID == -1) EXIT_ON_ERROR("Failed to create queue\n");

    setClient();

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

void setClient(){
    msg_t msg;
    msg.request_pid = getpid();
    msg.mtype = BEGIN;
    
    if(mq_send(queueID, (char*)&msg, MSG_SIZE, 1) == -1) EXIT_ON_ERROR("Couldn't send start request to server in pid: %d\n", getpid());
    if (mq_receive(clientID, (char*)&msg, MSG_SIZE, NULL) == -1) EXIT_ON_ERROR("Couldn't receive start permission request to server in pid: %d\n", getpid());
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
    if(mq_send(queueID, (char*)&msg, MSG_SIZE, 2) == -1) EXIT_ON_ERROR("Cannot send MIRROR request to server\n");
    if(mq_receive(clientID, (char*)&msg, MSG_SIZE, NULL) == -1) EXIT_ON_ERROR("Cannot receive MIRROR request from server ERRNO: %d\n", errno);
    printf("%s\n", msg->buffer);
}

void fcalc(msg_t *msg){
    msg->mtype = CALC;
    if(input_mode) {
        printf("Enter expression to calculate: ");
        if (fgets(msg->buffer, MSG_SIZE, stdin) == NULL) printf("Expression is too long\n");
    }
    if(mq_send(queueID, (char*)&msg, MSG_SIZE, 2) == -1) EXIT_ON_ERROR("Cannot send CALC request to server\n");
    if(mq_receive(clientID, (char*)&msg, MSG_SIZE, NULL) == -1) EXIT_ON_ERROR("Cannot receive CALC request from server\n");
    printf("%s\n", msg->buffer);
}

void ftime(msg_t *msg){
    msg->mtype = TIME;
    if(mq_send(queueID, (char*)&msg, MSG_SIZE, 2) == -1) EXIT_ON_ERROR("Cannot send TIME request to server\n");
    if(mq_receive(clientID, (char*)&msg, MSG_SIZE, NULL) == -1) EXIT_ON_ERROR("Cannot receive TIME request from server\n");
    printf("%s\n", msg->buffer);
}

void fend(msg_t *msg){
    msg->mtype = END;
    if(mq_send(queueID, (char*)&msg, MSG_SIZE, 2) == -1) EXIT_ON_ERROR("Cannot send END request to server\n");
}
