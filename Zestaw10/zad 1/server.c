#include "message.h"


void init(int , char *);

void *network_handling_thread(void*);
void accept_connection(int);
void handle_message(int);

void *ping_thread(void *);

void *terminal_handling_thread(void *);
void send_message(int, message *, int);

void register_client(char *, int);
void unregister_client(char *);
void delete_client(int);

void delete_socket(int);
void clean();

//////////////////////////////////////////////////////////////////////////////////////////////////

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
    srand(time(NULL));

    if(argc != 3) { printf("EXPECTED ARGUMENTS: [PORT NUMBER] [PORT PATH]\n"); return 1; }

    signal(SIGINT, clean);
    atexit(clean);

    init(atoi(argv[1]), argv[2]);


    if( pthread_create(&command, NULL, terminal_handling_thread, NULL) != 0 ){ perror("command pthread_create error"); exit(1); }
    if( pthread_create(&network, NULL, network_handling_thread, NULL) != 0 ) { perror("network pthread_create error"); exit(1); }
    if( pthread_create(&ping, NULL, ping_thread, NULL) != 0 )                { perror("ping pthread_create error"); exit(1); }


    pthread_join(command, NULL);
    pthread_join(network, NULL);
    pthread_join(ping, NULL);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void init(int port_num, char *path) {

    for (i = 0; i < CLIENT_MAX; i++)
        clients[i].reserved = -1;

    local_path = path;

    // AF_UNIX
    struct sockaddr_un* un_socket_addr;
    un_socket_addr = calloc(1, sizeof(struct sockaddr_un));
    un_socket_addr->sun_family = AF_UNIX;
    sprintf(un_socket_addr->sun_path, "%s", path);

    un_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (un_socket == -1){
        perror("un_socket: socket() error");
        exit(1);
    }
    if (bind(un_socket, (const struct sockaddr *)un_socket_addr, sizeof(*un_socket_addr))){
        perror("un_socket: bind() error");
        exit(1);
    }
    if (listen(un_socket, 64)){
        perror("un_socket: listen() error");
        exit(1);
    }


    // AF_INET
    struct sockaddr_in *in_socket_addr;
    in_socket_addr = calloc(1, sizeof(struct sockaddr_in));
    in_socket_addr->sin_family = AF_INET;
    in_socket_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    in_socket_addr->sin_port = htons(port_num);

    in_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (in_socket == -1){
        perror("in_socket: socket() error");
        exit(1);
    }
    if (bind(in_socket, (const struct sockaddr *)in_socket_addr, sizeof(*in_socket_addr))){
        perror("in_socket: bind() error");
        exit(1);
    }
    if (listen(in_socket, 64)){
        perror("in_socket: listen() error");
        exit(1);
    }


    // MONITOROWANIE WIELU DESKRYPTORÓW (EPOLL)
    epoll = epoll_create1(0);
    if(epoll == -1) { perror("epoll: epoll_create1() error"); exit(1); }

    struct epoll_event epoll_event;
    epoll_event.events = EPOLLIN | EPOLLPRI;

    epoll_event.data.fd = -un_socket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, un_socket, &epoll_event) == -1){
        perror("epoll: epoll_ctl() error for un_socket");
        exit(1);
    }

    epoll_event.data.fd = -in_socket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, in_socket, &epoll_event) == -1) {
        perror("epoll: epoll_ctl() error for in_socket");
        exit(1);
    }
}



void register_client(char *client_name, int socket) {
    uint8_t message_type;
    pthread_mutex_lock(&mutex);


    // if there is max clients
    if (clients_amount == CLIENT_MAX) {
        message_type = FAILURE;

        if ( send(socket, &message_type, 1, 0) == -1) { perror("register_client: send FAILURE 1 error"); exit(1); }
        delete_socket(socket);
    }

    // if client exists
    for(int i = 0; i < clients_amount; i++) {
        if(strcmp(client_name, clients[i].name) == 0){
            message_type = FAILURE;
            if (send(socket, &message_type, 1, 0) == -1) { perror("register_client: send FAILURE 2 error"); exit(1); }
            delete_socket(socket);
            break;
        }
    }

    // add client
    clients[clients_amount].fd = socket;
    clients[clients_amount].active_counter = 0;
    clients[clients_amount].reserved = 0;
    clients[clients_amount].name = calloc(strlen(client_name) + 1, sizeof(char));
    strcpy(clients[clients_amount].name, client_name);
    clients_amount++;

    message_type = SUCCESS;
    if( send(socket, &message_type, 1, 0) == -1 ) { perror("register_client: send SUCCESS error"); exit(1); }


    pthread_mutex_unlock(&mutex);
}



void *network_handling_thread(void* arg) {
    struct epoll_event event;

    while (1) {
        if (epoll_wait(epoll, &event, 1, -1) == -1) { perror("epoll_wait failed"); exit(1); }

        if (event.data.fd < 0)
            accept_connection(-event.data.fd);
        else
            handle_message(event.data.fd);
    }
}



void accept_connection(int socket) {
    int client_fd = accept(socket, NULL, NULL);
    if (client_fd == -1) { perror("network_handling_thread: accept_connection: accept() error"); exit(1); }

    struct epoll_event epoll_event;
    epoll_event.events = EPOLLIN;
    epoll_event.data.fd = client_fd;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, client_fd, &epoll_event) == -1)
        perror("network_handling_thread: epoll_ctl() error");
}



void handle_message(int socket) {
    uint8_t message_type;
    uint16_t message_size;

    if( recv(socket, &message_type, TYPE_SIZE, 0) == -1 ) { perror("handle_message: recv message_type error");  exit(1); }
    if( recv(socket, &message_size, LEN_SIZE, 0) == -1 )  { perror("handle_message: recv message_size error");  exit(1); }

    char *client_name = malloc(message_size);

    switch (message_type) {

        case REGISTER:
            if( recv(socket, client_name, message_size, 0) == -1 )  { perror("handle_message: REGISTER recv error");  exit(1); }

            register_client(client_name, socket);
            break;


        case UNREGISTER:
            if( recv(socket, client_name, message_size, 0) == -1 )  { perror("handle_message: UNREGISTER recv error");  exit(1); }

            unregister_client(client_name);
            break;


        case RESULT:
            printf("Received result \n");

            if( recv(socket, client_name, message_size, 0) == -1 ) { perror("handle_message: RESULT recv client_name error");  exit(1); }

            int size;
            if( recv(socket, &size, sizeof(int), 0) == -1) { perror("handle_message: RESULT recv size error");  exit(1); }

            char* result = calloc(size, sizeof(char));
            if( recv(socket, result, size, 0) == -1) { perror("handle_message: RESULT recv result error");  exit(1); }

            printf("Client: %s \nResult: \n%s", client_name, result);

            for(int i = 0; i < clients_amount; i++){
                if(strcmp(client_name, clients[i].name) == 0){
                    clients[i].reserved --;
                    clients[i].active_counter = 0;
                }
            }

            free(result);

            break;


        case PING:
            if( recv(socket, client_name, message_size, 0) == -1 ) { perror("handle_message: PING recv error");  exit(1); }

            pthread_mutex_lock(&mutex);

            int index = -1;
            for(int i = 0; i < clients_amount; i++)
                if(strcmp(client_name, clients[i].name) == 0)
                    index = i;

            if (index >= 0) clients[index].active_counter = clients[index].active_counter == 0 ? 0 : clients[index].active_counter-1;
            pthread_mutex_unlock(&mutex);

            break;


        default:
            printf("handle_message: message type %d unknown\n", message_type);
            break;
    }
    free(client_name);
}



void *ping_thread(void *arg) {
    uint8_t message_type = PING;

    while (1) {
        pthread_mutex_lock(&mutex);

        for (int i = 0; i < clients_amount; ++i) {
            if (clients[i].active_counter) {
                printf("Client \"%s\" is not responding.\n", clients[i].name);
                delete_client(i);
            }
            else {
                if (send(clients[i].fd, &message_type, 1, 0) != 1) { perror("ping_thread: send error"); exit(1); }
                clients[i].active_counter++;
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(5);
    }
}



void send_message(int type, message *msg, int client_nr) {

    if( send(clients[client_nr].fd, &type, 1, 0) != 1)  { perror("send_message: send type error"); return; }

    int len = sizeof(*msg);
    if( send(clients[client_nr].fd, &len, 2, 0) != 2) { perror("send_message: send length error"); return; }

    if( send(clients[client_nr].fd, msg, len, 0) != len)  { perror("send_message: send message error"); return; }

}



void *terminal_handling_thread(void *arg) {

    while (1) {
        printf("Enter file name: \n");

        char *file_name = calloc(MAX_PATH_LENGTH, sizeof(char));
        scanf("%s", file_name);

        FILE *file;
        if ((file = fopen(file_name, "r")) == NULL)  { perror("terminal_handling_thread: fopen error"); continue; }

        char *text = calloc(MESSAGE_SIZE, sizeof(char));
        size_t text_size = fread(text, sizeof(char), MESSAGE_SIZE, file);

        fclose(file);

        if(text_size == 0 )  { printf("file %s is empty", file_name); continue; }

        id++;
        message *request = calloc(1, sizeof(message));
        request->ID = id;
        strcpy(request->text, text);


        // send to first free client
        int i;
        for (i = 0; i < clients_amount; i++) {
            if (clients[i].reserved == 0) {
                clients[i].reserved++;
                send_message(REQUEST, request, i);
                printf("Request: %d send to client: %s \n", request->ID, clients[i].name);
                break;
            }
        }

        // if not send
        if (i == clients_amount) {
            i = rand() % clients_amount;

            printf("Request: %d send to client: %s \n", request->ID, clients[i].name);

            clients[i].reserved ++;
            send_message(REQUEST, request, i);
        }

        free(file_name);
        free(text);
        free(request);
    }
}



void unregister_client(char *client_name) {
    pthread_mutex_lock(&mutex);

    int index = -1;
    for(int i = 0; i < clients_amount; i++)
        if(strcmp(client_name, clients[i].name) == 0)
            index = i;

    if(index == -1)
        printf("unregister_client: client %s not found", client_name);
    else {
        delete_client(index);
        printf("Client %s unregistered\n", client_name);
    }

    pthread_mutex_unlock(&mutex);
}



void delete_client(int i) {

    delete_socket(clients[i].fd);
    free(clients[i].name);

    for (int j = i; j < clients_amount; ++j)
        clients[j] = clients[j + 1];

    clients_amount--;

}



void delete_socket(int socket) {
    if (epoll_ctl(epoll, EPOLL_CTL_DEL, socket, NULL) == -1) perror("delete_socket: epoll_ctl error");

    if (shutdown(socket, SHUT_RDWR) == -1) perror("delete_socket: shutdown error");

    if (close(socket) == -1) perror("delete_socket: close error");
}



void clean() {
    pthread_cancel(ping);
    pthread_cancel(command);
    pthread_cancel(network);

    if( close(in_socket) == -1 )   perror("clean: close in_socket error");
    if( close(un_socket) == -1 )   perror("clean: close un_socket error");
    if( unlink(local_path) == -1 ) perror("clean: unlink error");
    if( close(epoll) == -1 )       perror("clean: close epoll error");
}

