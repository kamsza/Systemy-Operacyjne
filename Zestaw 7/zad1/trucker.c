#include "line.h"

Belt *fifo = NULL;

int memID = -1;
int semID = -1;

int max_amount;
int max_weight;
int max_quantity;
int curr_amount;

Box box;
int status;
int semTaken = 0;

void intHandler(int);

void clear(void);

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
    char *path = getenv("HOME");
    if (path == NULL) { perror("ERROR getenv:"); exit(1); };

    key_t key = ftok(path, PROJECT_ID);
    if (key == -1) { perror("ERROR ftok:"); exit(1); };

    memID = shmget(key, sizeof(Belt), IPC_CREAT | IPC_EXCL | 0666);
    if (memID == -1) { perror("ERROR shmget:"); exit(1); }

    void *tmp = shmat(memID, NULL, 0);
    if (tmp == (void *) (-1)) { perror("ERROR shmat:"); exit(1); }
    fifo = (Belt *) tmp;

    init_belt(fifo, max_quantity, max_weight);

    // PREPARE SEMAPHORES
    semID = semget(key, 4, IPC_CREAT | IPC_EXCL | 0666);
    if (semID == -1) { perror("ERROR semget:"); exit(1); }

    for (int i = 1; i < 3; i++)
        if (semctl(semID, i, SETVAL, 1) == -1) { perror("ERROR semctl:"); exit(1); };

    if (semctl(semID, TRUCKER, SETVAL, 0) == -1) { perror("ERROR semctl TRUCKER:"); exit(1); };
}

void load_boxes(){
    struct sembuf sops;
    sops.sem_flg = 0;
    while (1) {
        sops.sem_num = TRUCKER;
        sops.sem_op = -1;

        if (semop(semID, &sops, 1) == -1) { perror("ERROR semop TRUCKER:"); exit(1); };

        sops.sem_num = BELT;
        sops.sem_op = -1;
        if (semop(semID, &sops, 1) == -1) { perror("ERROR semop BELT:"); exit(1); };
        semTaken = 1;

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
        sops.sem_num = BELT;
        sops.sem_op = 1;
        if (semop(semID, &sops, 1) == -1) { perror("ERROR semop BELT:"); exit(1); };
        semTaken = 0;

        sops.sem_num = LOADERS;
        sops.sem_op = 1;
        if (semop(semID, &sops, 1) == -1) { perror("ERROR semop LOADERS:"); exit(1); };
    }

}
void unload_truck(){
    curr_amount = 0;
    sleep(1);
}



void clear() {
	printf("\n");
    if(!semTaken){
        struct sembuf sops;
        sops.sem_flg = 0;
        sops.sem_num = BELT;
        sops.sem_op = -1;
        if (semop(semID, &sops, 1) == -1) { perror("ERROR semop BELT in clear function:"); exit(1); };
    }
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

    if (shmdt(fifo) == -1)  { perror("ERROR shmdt fifo:"); exit(1); };
    if (shmctl(memID, IPC_RMID, NULL) == -1) { perror("ERROR shmctl memID:"); exit(1); };
    if (semctl(semID, 0, IPC_RMID) == -1) { perror("ERROR shmctl semID:"); exit(1); };
}

void intHandler(int signo) {
    exit(2);
}
