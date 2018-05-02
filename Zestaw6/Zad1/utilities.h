#ifndef _UTILITIES_H
#define _UTILITIES_H

#define PROJECT_ID 7312
#define MAX_CLIENTS 10
#define MSG_SIZE 128

typedef struct{
    long mtype;
    pid_t request_pid;
    char buffer[MSG_SIZE];
}msg_t;

typedef enum M_TYPE{
    BEGIN = 1, MIRROR = 2, CALC = 3, TIME = 4, END = 5, STOP = 6
}M_TYPE;

#endif
