#include "line.h"


Belt *fifo = NULL;


int max_amount;
int max_weight;
int max_quantity;
int curr_amount;

int status;
int queue;

Box box;

sem_t *trucker_sem;
sem_t *loaders_sem;
sem_t *belt_sem;


void intHandler(int);

void clear();

void init(int max_weight, int max_items);

void load_boxes();

void unload_truck();

int main(int argc, char **argv){
    if(argc != 4) { printf("Wrong arguments - expected: <truck load> <conveyor belt load> <conveyor belt max weight>"); return 1; };

    max_amount = atoi(argv[1]);
    if (max_amount < 1) { printf("Wrong arguments - truck load can not be negative"); return 1; };
    
    max_quantity = atoi(argv[2]);
    if (max_quantity < 1) { printf("Wrong arguments - conveyor belt load can not be negative"); return 1; };
    
    max_weight = atoi(argv[3]);
    if (max_weight < 1) { printf("Wrong arguments - conveyor belt max weight can not be negative"); return 1; };

    if (atexit(clear) == -1) { perror("ERROR atexit:"); return 1; }
    if (signal(SIGINT, intHandler) == SIG_ERR) { perror("ERROR signal:"); return 1; }

    init(max_weight, max_quantity);

    load_boxes();
}



void init(int max_weight, int max_items) {
    // PREPARE MEMORY
    int memID = shm_open("conveyor belt", O_CREAT | O_EXCL | O_RDWR, 0666);
    if (memID == -1) { perror("ERROR shm_open"); exit(1); }

    if (ftruncate(memID, sizeof(Belt)) == -1) { perror("ERROR ftruncate"); exit(1); }

    void *tmp = mmap(NULL, sizeof(Belt), PROT_READ | PROT_WRITE, MAP_SHARED, memID, 0);
    if (tmp == (void *) (-1)) { perror("ERROR mmap"); exit(1); }
    fifo = (Belt *) tmp;

    init_belt(fifo, max_quantity, max_weight);

    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg = 10;
    queue_attr.mq_msgsize = 10;

    if ((queue = mq_open("/pidQueue",O_CREAT | O_EXCL | O_RDONLY | O_NONBLOCK, 0666, &queue_attr)) == -1) { perror("ERROR mq_open"); exit(1); }
    
    // PREPARE SEMAPHORES
    trucker_sem = sem_open("trucker_sem", O_CREAT | O_EXCL | O_RDWR, 0666, 0);
    if (trucker_sem == SEM_FAILED) { perror("ERROR sem_open for trucker_sem:"); exit(1); }

    loaders_sem = sem_open("loaders_sem",O_CREAT | O_EXCL | O_RDWR, 0666, 1);
    if (loaders_sem == SEM_FAILED) { perror("ERROR sem_open for loaders_sem:"); exit(1); }

    belt_sem = sem_open("belt_sem",O_CREAT | O_EXCL | O_RDWR, 0666, 1);
    if (belt_sem == SEM_FAILED)  { perror("ERROR sem_open for belt_sem:"); exit(1); }

}




void load_boxes(){
    while (1) {
        if (sem_wait(trucker_sem) == -1) { perror("ERROR sem_wait for trucker_sem:"); return; }
        if (sem_wait(belt_sem) == -1) { perror("ERROR sem_wait for belt_sem:"); return; }

        status = pop(fifo, &box);
        while(status == 0){
            if(curr_amount == max_amount) {
                printf(COLOR_BLUE "TRUCKER:" COLOR_RESET " truck full \n");
                unload_truck();
                printf(COLOR_BLUE "TRUCKER:" COLOR_RESET " new truck arrived \n");
            }
            curr_amount++;
            printf(COLOR_BLUE "TRUCKER:" COLOR_RESET " box loaded   weight: %d   pid: %d   time diff: %ld    Place left: %d \n", box.weight,box.pid,get_time()-box.time,max_amount-curr_amount);
            status = pop(fifo, &box);
        }
        if (sem_post(belt_sem) == -1) { perror("ERROR sem_post for belt_sem:"); return; }

        if (sem_post(loaders_sem) == -1) { perror("ERROR sem_post for loaders_sem:"); return; }
    }

}


void unload_truck(){
    sleep(1);
    curr_amount = 0;
}


void clear() {
	printf("\n");
	
    char pid[10];
    int i;
    while (mq_receive(queue, pid, 10, NULL) != -1) {
        i = strtol(pid, NULL, 10);
        kill(i, SIGINT);
    }

    if (mq_close(queue) == -1) perror("ERROR mq_close:");

    if (mq_unlink("/pidQueue") == -1) perror("ERROR mq_unlink:");

    if (belt_sem != NULL && fifo != NULL) {
        status = pop(fifo, &box);
        while (status == 0) {
            if (curr_amount == max_amount) {
                printf(COLOR_BLUE "TRUCKER:" COLOR_RESET " truck full \n");
                unload_truck();
                printf(COLOR_BLUE "TRUCKER:" COLOR_RESET " new truck arrived \n");
            }
            curr_amount++;
            printf(COLOR_BLUE "TRUCKER:" COLOR_RESET " box loaded   weight: %d   pid: %d   time diff: %ld    Place left: %d \n",
                   box.weight, box.pid, get_time() - box.time, max_amount - curr_amount);
            status = pop(fifo, &box);
        }
    }

    if (fifo != NULL) {
        if (munmap(fifo, sizeof(fifo)) == -1) perror("ERROR munmap:");

        if (shm_unlink("conveyor belt") == -1) perror("ERROR shm_unlink:");

        if (trucker_sem != NULL)
            if (sem_close(trucker_sem) == -1) perror("ERROR sem_close for trucker_sem:");
        if (sem_unlink("trucker_sem") == -1) perror("ERROR sem_unlink for trucker_sem:");

        if (loaders_sem != NULL)
            if (sem_close(loaders_sem) == -1) perror("ERROR sem_close for loaders_sem:");
        if (sem_unlink("loaders_sem") == -1) perror("ERROR sem_unlink for loaders_sem:");

        if (belt_sem != NULL)
            if (sem_close(belt_sem) == -1) perror("ERROR sem_close for belt_sem:");
        if (sem_unlink("belt_sem") == -1) perror("ERROR sem_unlink for belt_sem:");
    }
}

void intHandler(int signo) {
    exit(2);
}
