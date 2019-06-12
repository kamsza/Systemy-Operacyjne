#ifndef SOCKETS_MSG_H
#define SOCKETS_MSG_H


#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <endian.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>


#define MESSAGE_SIZE 60000
#define MAX_PATH_LENGTH 64
#define CLIENT_MAX 12
#define TYPE_SIZE 1
#define LEN_SIZE 2


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr_un sockaddr_un;
typedef struct epoll_event epoll_event;


typedef enum message_type {
    REGISTER ,
    UNREGISTER,

    REQUEST,
    RESULT,

    SUCCESS,
    FAILURE,

    PING,
} message_type;


typedef struct Client {
    int fd;
    char *name;
    int active_counter;
    int reserved;
} Client;


typedef struct message{
    char text[MESSAGE_SIZE];
    int ID;
} message;

#endif //SOCKETS_MSG_H
