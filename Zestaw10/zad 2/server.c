#include "message.h"


void init(int , char *);

void *network_handling_thread(void*);
void handle_message(int);

void *ping_thread(void *);

void *terminal_handling_thread(void *);
void send_message(int, message *, int);

void register_client(int, message, struct sockaddr *, socklen_t);
void unregister_client(int);

void delete_socket(int);
void clean();


int in_socket;
int un_socket;

char *local_path;
int epoll;
int id;

pthread_t ping;
pthread_t command;
pthread_t network;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Client clients[CLIENT_MAX];
int clients_amount = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    if(argc != 3) { printf("EXPECTED ARGUMENTS: [PORT NUMBER] [PORT PATH]\n"); return 1; }

    signal(SIGINT, clean);

    init(atoi(argv[1]), argv[2]);


    if( pthread_create(&ping, NULL, ping_thread, NULL) != 0 )                { perror("ping pthread_create error"); exit(1); }
    if( pthread_create(&command, NULL, terminal_handling_thread, NULL) != 0 ){ perror("command pthread_create error"); exit(1); }
    if( pthread_create(&network, NULL, network_handling_thread, NULL) != 0 ) { perror("network pthread_create error"); exit(1); }


    pthread_join(ping, NULL);
    pthread_join(command, NULL);
    pthread_join(network, NULL);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////


void init(int port_num, char *path)  {


    for (int i = 0; i < CLIENT_MAX; i++)
        clients[i].reserved = -1;

    local_path = path;

    // AF_UNIX
    struct sockaddr_un* un_socket_addr;
    un_socket_addr = calloc(1, sizeof(struct sockaddr_un));
    un_socket_addr->sun_family = AF_UNIX;
    sprintf(un_socket_addr->sun_path, "%s", path);

    un_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (un_socket == -1){
        perror("un_socket: socket() error");
        exit(1);
    }

    if (bind(un_socket, (const struct sockaddr *) un_socket_addr, sizeof(*un_socket_addr))){
        perror("un_socket: bind() error");
        exit(1);
    }

    // AF_INET
    struct sockaddr_in *in_socket_addr;
    in_socket_addr = calloc(1, sizeof(struct sockaddr_in));
    in_socket_addr->sin_family = AF_INET;
    in_socket_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    in_socket_addr->sin_port = htons(port_num);

    in_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (in_socket == -1){
        perror("in_socket: socket() error");
        exit(1);
    }

    if (bind(in_socket, (const struct sockaddr *) in_socket_addr, sizeof(*in_socket_addr))){
        perror("in_socket: bind() error");
        exit(1);
    }


    // MONITOROWANIE WIELU DESKRYPTORÓW (EPOLL)
    epoll = epoll_create1(0);
    if(epoll == -1) { perror("epoll: epoll_create1() error"); exit(1); }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;

    event.data.fd = in_socket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, in_socket, &event) == -1){
        perror("epoll: epoll_ctl() error for in_socket");
        exit(1);
    }
    event.data.fd = un_socket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, un_socket, &event) == -1){
        perror("epoll: epoll_ctl() error for un_socket");
        exit(1);
    }

    printf("Server is working\n");
}



void register_client(int socket, message msg, struct sockaddr *sockaddr, socklen_t socklen) {

    uint8_t message_type;

    pthread_mutex_lock(&mutex);

    // if there is max clients
    if (clients_amount == CLIENT_MAX) {
        message_type = FAILURE;
        if (sendto(socket,& message_type, 1, 0, sockaddr, socklen)  == -1)
            perror("register_client: sendto failure 1 message error");
        return;
    }

    // if client exists
    for(int i = 0; i < clients_amount; i++) {
        if(strcmp(msg.client, clients[i].name) == 0){
            message_type = FAILURE;
            if (sendto(socket, &message_type, 1, 0, sockaddr, socklen)  == -1)
                perror("register_client: sendto failure 2 message error");
            return;
        }
    }

    // add client
    clients[clients_amount].name = calloc(strlen(msg.client) + 1, sizeof(char));
    strcpy(clients[clients_amount].name, msg.client);
    clients[clients_amount].active_counter = 0;
    clients[clients_amount].reserved = 0;

    clients[clients_amount].socket = socket;
    clients[clients_amount].socklen = socklen;
    clients[clients_amount].sockaddr = malloc(sizeof(struct sockaddr_un));
    memcpy(clients[clients_amount].sockaddr, sockaddr, socklen);

    clients_amount++;

    printf("client %s registered\n", msg.client);

    message_type = SUCCESS;
    if (sendto(socket, &message_type, 1, 0, sockaddr, socklen)  == -1){perror("register_client: sendto failure message error"); return;}

    pthread_mutex_unlock(&mutex);
}



void *network_handling_thread(void* arg) {
    struct epoll_event event;

    while (1) {
        if (epoll_wait(epoll, &event, 1, -1) == -1) { perror("epoll_wait failed"); exit(1); }

        handle_message(event.data.fd);
    }
}



void handle_message(int socket) {

    sockaddr *sockaddr = calloc(1, sizeof(sockaddr));
    socklen_t socklen = sizeof(sockaddr);

    int i = 0;
    message msg;

    if (recvfrom(socket, &msg, sizeof(message), 0, sockaddr, &socklen) == -1) { perror("handle_message: recvfrom error"); return; }

    switch (msg.message_type) {

        case REGISTER:
            register_client(socket, msg, sockaddr, socklen);
            break;


        case UNREGISTER:
            // find client by name
            for (i = 0; i < clients_amount; i++)
                if (strcmp(clients[i].name, msg.client) == 0) {
                    unregister_client(i);
                    break;
                }

            if(i == clients_amount)
                printf("Client %s not found", msg.client);

            break;


        case RESULT:
            pthread_mutex_lock(&mutex);
            printf("Received result %d\n", msg.ID);

            // find client by name
            for (int i = 0; i < clients_amount; i++)
                if (strcmp(clients[i].name, msg.client) == 0) {
                    clients[i].reserved--;
                    clients[i].active_counter = 0;
                }

            printf("Result: %s \n", msg.text);
            printf("From: %s \n", msg.client);
            pthread_mutex_unlock(&mutex);
            break;


        case PING:
            pthread_mutex_lock(&mutex);

            int index = -1;
            for(int i = 0; i < clients_amount; i++)
                if(strcmp(msg.client, clients[i].name) == 0)
                    index = i;

            if (index >= 0) clients[index].active_counter--;

            pthread_mutex_unlock(&mutex);
            break;


        default:
            printf("handle_message: message from socket %d message type unknown\n", socket);
            break;
    }
}



void *ping_thread(void *arg) {
    message msg;
    msg.message_type = PING;

    while (1) {
        pthread_mutex_lock(&mutex);

        for (int i = 0; i < clients_amount; i++) {

            if (clients[i].active_counter) {
                printf("client %s is not active\n", clients[i].name);
                unregister_client(i);
            }

            else {
                msg.ID = id;
                id++;

                if (sendto(clients[i].socket, &msg, sizeof(message), 0, clients[i].sockaddr, clients[i].socklen) == -1) {
                    perror("ping_thread: sendto error");
                    exit(1);
                }

                clients[i].active_counter++;
            }
        }

        pthread_mutex_unlock(&mutex);
        sleep(5);
    }
}



void *terminal_handling_thread(void *arg) {

    while (1) {
        printf("Enter file name: \n");

        char *file_name = calloc(MAX_PATH_LENGTH, sizeof(char));
        scanf("%s", file_name);

        if(clients_amount == 0) { printf("Can not handle request: 0 active clients"); continue;}

        FILE *file;
        if ((file = fopen(file_name, "r")) == NULL)  { perror("terminal_handling_thread: fopen error"); continue; }

        char *text = calloc(MESSAGE_SIZE, sizeof(char));
        size_t text_size = fread(text, sizeof(char), MESSAGE_SIZE, file);

        fclose(file);

        if(text_size == 0 )  { printf("file %s is empty", file_name); continue; }




        // send to first free client
        message *request = calloc(1, sizeof(message));
        request->ID = id;
        request->message_type = REQUEST;
        strcpy(request->text, text);
        id++;


        int i;
        int index = -1;
        for (i = 0; i < clients_amount; i++) {
            if (clients[i].reserved == 0) {
                index = i;
                break;
            }
        }

        // if not send
        if (index == -1)
            index = rand() % clients_amount;

        if (sendto(clients[index].socket, request, sizeof(message), 0, clients[index].sockaddr, clients[index].socklen) == -1){
            perror("ping_thread: send error");
            exit(1);
        }

        clients[index].reserved ++;
        printf("Request: %d send to client: %s \n", request->ID, clients[i].name);



        free(file_name);
        free(text);
        free(request);
    }
}



void unregister_client(int index) {
    pthread_mutex_lock(&mutex);

    message *msg = calloc(1, sizeof(message));
    msg->message_type = UNREGISTER;
    msg->ID = id;
    id ++;

    if (sendto(clients[index].socket, msg, sizeof(message), 0, clients[index].sockaddr, clients[index].socklen) == -1) {
        perror("clean: sendto error");
    }

    free(clients[index].sockaddr);
    free(clients[index].name);

    for (int j = index; j < clients_amount; ++j)
        clients[j] = clients[j + 1];

    clients_amount--;

    printf("client %d unregistered", index);

    pthread_mutex_unlock(&mutex);
}



void clean() {
    pthread_mutex_lock(&mutex);

    pthread_cancel(ping);
    pthread_cancel(command);

    for (int i = clients_amount - 1; i >= 0; i--)
        unregister_client(i);

    if (close(in_socket) == -1)
        perror("Could not close Web Socket");
    if (close(un_socket) == -1)
        perror("Could not close Local Socket");
    if (unlink(local_path) == -1)
        perror("Could not unlink Unix Path");
    if (close(epoll) == -1)
        perror("Could not close epoll");



    pthread_cancel(network);

    pthread_mutex_unlock(&mutex);
}


