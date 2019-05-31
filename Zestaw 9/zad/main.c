#include "main.h"

int current_car_id = 0;
Car *current_car = NULL;


void *car_function(void *arg);

void *passenger_function(void *arg);

char *get_time();

void init();

void clear();


int main(int argc, char *argv[]) {

    if (argc != 5) {
        printf("Wrong arguments - expected <number of passengers> <number of cars> <car capacity> <number of runs>\n");
        return 1;
    }

    printf("======================================= ROLLER COASTER =======================================\n");

    passengers_num = atoi(argv[1]);
    cars_num = atoi(argv[2]);
    cars_working = cars_num;
    car_capacity = atoi(argv[3]);
    runs_num = atoi(argv[4]);

    if (passengers_num <= 0 || cars_working <= 0 || car_capacity <= 0 || runs_num <= 0) {
        printf("Wrong arguments - all values must be greater than 0 \n");
        return 1;
    }

    init();

    int *arg;
    for (int i = 0; i < passengers_num; i++) {
        arg = malloc(sizeof(int));
        *arg = i;
        if (pthread_create(&passenger_thread[i], NULL, passenger_function, (void *) arg))
            perror("pthread_create error for passenger");
    }

    for (int i = 0; i < cars_working; i++) {
        arg = malloc(sizeof(int));
        *arg = i;
        if (pthread_create(&car_thread[i], NULL, car_function, (void *) arg)) perror("pthread_create error for car");
    }


    for (int i = 0; i < cars_working; i++)
        if (pthread_join(car_thread[i], NULL)) perror("pthread_join error for car");


    for (int i = 0; i < passengers_num; i++)
        if (pthread_join(passenger_thread[i], NULL)) perror("pthread_join error for passenger");


    clear();

    return 0;
}


///////////////////////////////  CAR  ///////////////////////////////

void *car_function(void *arg) {

    //INIT
    Car this;
    this.id = *(int *) arg;
    this.empty_seats = car_capacity;
    this.runs_left = runs_num + 1;

    printf(COLOR_BLUE "[%s] Car: %2d" COLOR_RESET " start working\n", get_time(), this.id);

    while (this.runs_left--) {

        // ARRIVING AT THE STATION
        pthread_mutex_lock(&car_on_station_mutex);

        while (this.id != current_car_id)
            pthread_cond_wait(&car_order_cond, &car_on_station_mutex);


        printf(COLOR_BLUE "\n[%s] Car: %2d" COLOR_RESET " arrive at station\n", get_time(), this.id);

        current_car = &this;

        printf(COLOR_BLUE "[%s] Car: %2d" COLOR_RESET " doors opened \n", get_time(), this.id);

        pthread_mutex_lock(&passenger_mutex);

        // LEAVING PASSENGERS
        if (this.empty_seats < car_capacity) {
            pthread_cond_signal(&leaving_passengers);
            pthread_cond_wait(&car_empty, &passenger_mutex);
        }

        // GETTING NEW PASSENGERS
        if (this.runs_left != 0) {
            pthread_cond_signal(&getting_passengers);
            pthread_cond_wait(&car_full, &passenger_mutex);
        }
        pthread_mutex_unlock(&passenger_mutex);

        printf(COLOR_BLUE "[%s] Car: %2d" COLOR_RESET " doors closed \n", get_time(), this.id);

        // LEAVING STATION
        if (this.runs_left != 0) {
            printf(COLOR_BLUE "[%s] Car: %2d" COLOR_RESET " begin ride \n\n", get_time(), this.id);
            current_car_id = (current_car_id + 1) % cars_num;
            pthread_mutex_unlock(&car_on_station_mutex);
            pthread_cond_broadcast(&car_order_cond);
        }
    }

    // ENDING
    pthread_mutex_trylock(&car_on_station_mutex);
    current_car_id = (current_car_id + 1) % cars_num;

    cars_working--;
    printf(COLOR_BLUE "[%s] Car: %2d" COLOR_RESET " end his rides  cars left: %d \n\n", get_time(), this.id, cars_working);

    if (cars_working == 0)
        pthread_cond_broadcast(&getting_passengers);

    pthread_mutex_unlock(&car_on_station_mutex);
    pthread_cond_broadcast(&car_order_cond);

    return NULL;
}

////////////////////////////  PASSENGER  ////////////////////////////

void *passenger_function(void *arg) {

    //INIT
    Passenger this;
    this.id = *(int *) arg;
    this.car = NULL;
    printf(COLOR_YELLOW "[%s] Passenger: %2d" COLOR_RESET " start his rides\n", get_time(), this.id);

    while (cars_working) {

        // GETTING INTO A CAR
        pthread_mutex_lock(&passenger_mutex);
        pthread_cond_wait(&getting_passengers, &passenger_mutex);

        if (cars_working == 0) break;

        this.car = current_car;
        this.car->empty_seats--;

        printf(COLOR_YELLOW "[%s] Passenger: %2d" COLOR_RESET " entered the car %d   seats left: %d\n", get_time(),
                                                                      this.id, this.car->id, this.car->empty_seats);

        if (this.car->empty_seats == 0) {
            printf(COLOR_YELLOW "[%s] Passenger: %2d"  COLOR_RESET " pressed the START button\n", get_time(), this.id);
            pthread_cond_signal(&car_full);
        }
        else
            pthread_cond_signal(&getting_passengers);

        // LEAVING THE CAR
        pthread_cond_wait(&leaving_passengers, &passenger_mutex);

        this.car->empty_seats++;
        printf(COLOR_YELLOW "[%s] Passenger: %2d" COLOR_RESET " left the car %d   passengers in car: %d\n", get_time(),
               this.id, this.car->id, car_capacity - this.car->empty_seats);

        if (this.car->empty_seats ==
            car_capacity)                    // client sends signal he is the last one server can handle
            pthread_cond_signal(&car_empty);
        else
            pthread_cond_signal(&leaving_passengers);

        this.car = NULL;
        pthread_mutex_unlock(&passenger_mutex);

    }

    pthread_mutex_trylock(&passenger_mutex);
    printf(COLOR_YELLOW "[%s] Passenger: %2d" COLOR_RESET " finish his rides\n", get_time(), this.id);
    pthread_mutex_unlock(&passenger_mutex);

    return NULL;
}

/////////////////////////////////////////////////////////////////////

void init() {
    passenger_thread = calloc(passengers_num, sizeof(pthread_t));
    car_thread = calloc(cars_working, sizeof(pthread_t));

    if (pthread_mutex_init(&car_on_station_mutex, NULL) != 0) perror("pthread_mutex_init error");
    if (pthread_mutex_init(&passenger_mutex, NULL) != 0) perror("pthread_mutex_init error");


    if (pthread_cond_init(&car_order_cond, NULL) != 0) perror("pthread_cond_init error");
    if (pthread_cond_init(&car_empty, NULL) != 0) perror("pthread_cond_init error");
    if (pthread_cond_init(&car_full, NULL) != 0) perror("pthread_cond_init error");
    if (pthread_cond_init(&leaving_passengers, NULL) != 0) perror("pthread_cond_init error");
    if (pthread_cond_init(&getting_passengers, NULL) != 0) perror("pthread_cond_init error");
}

void clear() {
    if (pthread_mutex_destroy(&car_on_station_mutex) != 0) perror("pthread_mutex_destroy car_on_station_mutex error");
    if (pthread_mutex_destroy(&passenger_mutex) != 0) perror("pthread_mutex_destroy passenger_mutex error");


    if (pthread_cond_destroy(&car_order_cond) != 0) perror("pthread_cond_destroy car_order_cond error");
    if (pthread_cond_destroy(&car_empty) != 0) perror("pthread_cond_destroy car_empty error");
    if (pthread_cond_destroy(&car_full) != 0) perror("pthread_cond_destroy car_full error");
    if (pthread_cond_destroy(&leaving_passengers) != 0) perror("pthread_cond_destroy leaving_passengers error");
    if (pthread_cond_destroy(&getting_passengers) != 0) perror("pthread_cond_destroy getting_passengers error");

    free(passenger_thread);
    free(car_thread);
}

char *get_time() {
    char *time = malloc(13 * sizeof(char));
    struct timeval tv;
    gettimeofday(&tv, 0);
    time_t cur_time = tv.tv_sec;
    struct tm *t = localtime(&cur_time);
    sprintf(time, "%02d:%02d:%02d:%03ld", t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec / 1000);
    return time;
}
