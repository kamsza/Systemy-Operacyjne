//
// Created by kamil on 27.05.19.
//

#ifndef LAB9_MAIN_H
#define LAB9_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>


#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"


typedef struct Car{
    int id;
    int empty_seats;
    int runs_left;
}Car;


typedef struct Passenger{
    int id;
    Car* car;
} Passenger;


pthread_t *passenger_thread;
pthread_t *car_thread;


pthread_mutex_t car_on_station_mutex;
pthread_mutex_t passenger_mutex;


pthread_cond_t car_order_cond;
pthread_cond_t car_empty;
pthread_cond_t car_full;
pthread_cond_t leaving_passengers;
pthread_cond_t getting_passengers;


int cars_working;
int cars_num;
int car_capacity;
int runs_num;
int passengers_num;

#endif //LAB9_MAIN_H