#include "line.h"

Belt *belt = NULL;

key_t key;

int memID = -1;
int semID = -1;
int max_box_weight;
int boxes = -1;

void init();
void clear();
void intHandler(int sig_no);
void place_box(Box box);
char* now();


int main(int argc, char **argv){
    if(argc != 2 && argc != 3) { printf("Wrong arguments - expected: <max box weight> [<number of boxes>]"); return 1; };
    max_box_weight = atoi(argv[1]);

    if (max_box_weight < 1) { printf("Wrong arguments - box weight must be greater than 1"); return 1; };

    if (atexit(clear) == -1) perror("ERROR atexit:");
    if (signal(SIGINT, intHandler) == SIG_ERR) perror("ERROR signal:");

    init();

    int boxes = -1;

    if(argc == 3) {
        boxes = atoi(argv[2]);
        if (boxes < 1) {
            printf("Wrong arguments - number of boxes must be greater than 1");
            return 1;
        }
    }

    Box box;
    box.pid = getpid();

    while(boxes < 0 || --boxes )
        place_box(box);

    exit(0);
}

void place_box(Box box) {
        struct sembuf sops;
        sops.sem_num = LOADERS;
        sops.sem_op = -1;
        sops.sem_flg = 0;
        if (semop(semID, &sops, 1) == -1) perror("ERROR semop for LOADERS");

        sops.sem_num = BELT;
        if (semop(semID, &sops, 1) == -1) perror("ERROR semop for BELT");

        box.time = get_time();
        box.weight = 1 + rand() % (max_box_weight - 1);

        if(push(belt,box) == -1){
            printf(COLOR_RED "LOADER %d:" COLOR_RESET "Can not place box on conveyor belt \n", getpid());

            sops.sem_num = TRUCKER;
            sops.sem_op = 1;
            if (semop(semID, &sops, 1) == -1) perror("ERROR semop for TRUCKER");

            sops.sem_num = BELT;
            sops.sem_op = 1;
            if (semop(semID, &sops, 1) == -1) perror("ERROR semop for BELT");
        }
        else{
            printf(COLOR_YELLOW "LOADER %d: "COLOR_RESET "box placed - weight: %d  time: %s \n", getpid(), box.weight, now());

            sops.sem_num = BELT;
            sops.sem_op = 1;
            if (semop(semID, &sops, 1) == -1) perror("ERROR semop for BELT");

            sops.sem_num = LOADERS;
            sops.sem_op = 1;
            if (semop(semID, &sops, 1) == -1) perror("ERROR semop for LOADERS");
        }
        


}

void init() {
    // KEY
    char *path = getenv("HOME");
    if (path == NULL) { perror("ERROR getenv:"); exit(1); };

    key = ftok(path, PROJECT_ID);
    if (key == -1) { perror("ERROR ftok:"); exit(1); };
    
    // PREPARE MEMORY
    memID = shmget(key, 0, 0);
    if (memID == -1) { perror("ERROR shmget:"); exit(1); };

    void *tmp = shmat(memID, NULL, 0);
    if (tmp == (void *) (-1)) { perror("ERROR shmat:"); exit(1); };
    belt = (Belt *) tmp;
    
    // PREPARE SEMAPHORE
    semID = semget(key, 0, 0);
    if (semID == -1) { perror("ERROR semget:"); exit(1); };
}


void clear() {
    if (shmdt(belt) == -1) { perror("ERROR shmdt:"); exit(1); }

    printf(COLOR_YELLOW "LOADER %d: "COLOR_RESET " finished his job \n", getpid());
}

void intHandler(int sig_no) {
    exit(2);
}

char* now() {
	char *output = malloc(12);

	time_t rawtime;
   struct tm * timeinfo;
	
	time ( &rawtime );
   timeinfo = localtime ( &rawtime );

 	sprintf(output, "%d:%d:%d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	return output;
}	
