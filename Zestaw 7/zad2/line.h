#ifndef __LINE_H__
#define __LINE_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define PROJECT_ID 77

#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

#define ERROR "\x1b[31mERROR\x1b[0m"

typedef enum semType{
    TRUCKER = 0, BELT = 1, LOADERS = 2
}semType;

typedef struct Box{
    int weight;
    pid_t pid;
    long time;
}Box;

typedef struct Belt{
    int head;
    int tail;

    int curWeight;
    int curQuantity;

    int maxWeight;
    int maxQuantity;

    Box fifo[1024];
}Belt;

typedef struct pid_table{
    pid_t pids[1024];
    int size;
}pid_table;

int pid_tableInit(pid_table *table);

int init_belt(Belt *belt,int maxQuantity, int maxWeight);

int pop(Belt *belt,Box *box);

int push(Belt *belt, Box box);

long get_time();


#endif // __LINE_H__

