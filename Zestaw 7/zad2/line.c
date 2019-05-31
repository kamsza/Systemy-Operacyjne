#include "line.h"

int pid_tableInit(pid_table *table){
    int i;
    for(i = 0;i < 1024; i++) table->pids[i] = -1;
    table->size = 0;
    return 0;
}

int init_belt(Belt *belt, int maxQuantity, int maxWeight){
    belt->maxQuantity = maxQuantity;
    belt->maxWeight = maxWeight;
    belt->curWeight = 0;
    belt->curQuantity = 0;
    belt->head = -1;
    belt->tail = 0;
    return 0;
}

int is_empty(Belt *fifo) {
    if (fifo->head == -1) return 1;
    else return 0;
}

int is_full(Belt *fifo) {
    if (fifo->head == fifo->tail) return 1;
    else return 0;
}

int pop(Belt *fifo,Box *boxaddr) {
    if (is_empty(fifo) == 1) return -1;

    Box box = fifo->fifo[fifo->head++];

    if (fifo->head == fifo->maxQuantity) fifo->head = 0;

    if (fifo->head == fifo->tail) fifo->head = -1;

    fifo->curWeight -= box.weight;
    fifo->curQuantity--;
    *boxaddr = box;
    return 0;
}

int push(Belt *fifo, Box box) {
    if (fifo->curQuantity == fifo->maxQuantity || fifo->curWeight + box.weight > fifo->maxWeight) {
        return -1;
    }
    if (is_empty(fifo) == 1)
        fifo->head = fifo->tail = 0;

    fifo->fifo[fifo->tail++] = box;

    fifo->curWeight += box.weight;
    fifo->curQuantity++;
    if (fifo->tail == fifo->maxQuantity) fifo->tail = 0;
    return 0;
}

long get_time() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec*1000 + time.tv_nsec / 1000000;
}
