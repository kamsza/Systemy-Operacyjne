#include "line.h"

Belt *belt = NULL;

int max_box_weight;
int boxes;
int queue;
sem_t *trucker_sem;
sem_t *loaders_sem;
sem_t *belt_sem;


void init();
void clear();
void intHandler(int sig_no);
void place_box(Box box);
void write_PID();
char* now();

int main(int argc, char **argv){
    
    if(argc != 2 && argc != 3) { printf("Wrong arguments - expected: <max box weight> [<number of boxes>]"); return 1; };
    max_box_weight = atoi(argv[1]);

    if (max_box_weight < 1) { printf("Wrong arguments - box weight must be greater than 1"); return 1; };
    
    if (atexit(clear) == -1) perror("ERROR atexit:");
    if (signal(SIGINT, intHandler) == SIG_ERR) perror("ERROR signal:");

    init();

    write_PID();
    
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
    
    return 0;
}

void place_box(Box box) {
        if (sem_wait(loaders_sem) == -1) { perror(ERROR "sem_wait for loaders_sem in place_box:"); return; }

        if (sem_wait(belt_sem) == -1) { perror(ERROR "sem_wait for belt_sem in place_box:"); return; }

        box.weight = 1 + (int)rand() % (max_box_weight - 1);
        box.time = get_time();
        
        if(push(belt,box) == -1){
            printf(COLOR_RED "LOADER %d:" COLOR_RESET "Can not place box on conveyor belt \n", getpid());

            sem_post(trucker_sem);

            sem_post(belt_sem);
        }
        else{
            printf(COLOR_YELLOW "LOADER %d: "COLOR_RESET "box placed - weight: %d  time: %s \n", getpid(), box.weight, now());

            if (sem_post(belt_sem) == -1) { perror(ERROR "sem_post for belt_sem in place_box:"); return; }

            if (sem_post(loaders_sem) == -1) { perror(ERROR "sem_post for loaders_sem in place_box:"); return; }
        }

}


void init() {
    // GET KEY
    key_t key;
    char *path = getenv("HOME");
    if (path == NULL) { perror("ERROR getenv:"); exit(1); }

    key = ftok(path, PROJECT_ID);
    if (key == -1) { perror("ERROR ftok:"); exit(1); }

    // PREPARE MEMORY
    int memID = shm_open("conveyor belt", O_RDWR, 0666);
    if (memID == -1) { perror("ERROR shm_open:"); exit(1); }

    void *tmp = mmap(NULL, sizeof(Belt), PROT_READ | PROT_WRITE, MAP_SHARED, memID, 0);
    if (tmp == (void *) (-1)) { perror("ERROR mmap:"); exit(1); }
    belt = (Belt *) tmp;

    // PREPARE SEMAPHORES
    trucker_sem = sem_open("trucker_sem", O_RDWR);
    if (trucker_sem == SEM_FAILED) { perror("ERROR sem_open:"); exit(1); }

    loaders_sem = sem_open("loaders_sem", O_RDWR);
    if (loaders_sem == SEM_FAILED) { perror("ERROR sem_open:"); exit(1); }

    belt_sem = sem_open("belt_sem", O_RDWR);
    if (belt_sem == SEM_FAILED)  { perror("ERROR sem_open:"); exit(1); }
}


void write_PID(){
    if ((queue = mq_open("/pidQueue", O_WRONLY)) == -1)
    { perror("ERROR mq_open in write_PID:"); exit(1); }

    char PID[10];
    sprintf(PID,"%d",getpid());

    if (mq_send(queue, PID, 10, 1) != 0)
    { perror("ERROR mq_send in write_PID:"); exit(1); }

    if (mq_close(queue) == -1)
    { perror("ERROR mq_close in write_PID:"); exit(1); }
}



void clear() {
    if(belt != NULL)
    if (munmap(belt, sizeof(Belt)) == -1) { perror("ERROR munmap for belt:"); exit(1); }

    if(trucker_sem != NULL)
    if (sem_close(trucker_sem) == -1) { perror("ERROR sem_close for trucker_sem:"); exit(1); }

    if(loaders_sem != NULL)
    if (sem_close(loaders_sem) == -1) { perror("ERROR sem_close for loaders_sem:"); exit(1); }

    if(belt_sem != NULL)
    if (sem_close(belt_sem) == -1) { perror("ERROR sem_close for belt_sem:"); exit(1); }

    printf(COLOR_YELLOW "LOADER %d: "COLOR_RESET "finished his job\n", getpid());
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










