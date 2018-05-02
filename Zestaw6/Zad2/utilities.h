#ifndef _UTILITIES_H
#define _UTILITIES_H


#include <unistd.h>

#define PROJECT_ID 7312
#define MAX_CLIENTS 10
#define MSG_LIMIT 10
#define MAX_MSG_LENGTH 2048

typedef struct{
    int mtype;
    pid_t request_pid;
    char buffer[MAX_MSG_LENGTH];
}msg_t;

typedef enum M_TYPE{
    BEGIN = 1, MIRROR = 2, CALC = 3, TIME = 4, END = 5, STOP = 6
}M_TYPE;

const char qPath[] = "/server";

#define MSG_SIZE sizeof(msg_t)

#endif
