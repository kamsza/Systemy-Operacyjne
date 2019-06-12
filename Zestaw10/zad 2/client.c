#include "message.h"

char *name;
int sock_fd;
int run = 1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

////////////////////////////////////////////////////////////////////////////////////////

int get_unix_socket(const char*);
int get_inet_socket(int, const char*);

void send_register_request();

void message_handling_loop();
void* handle_message(void * arg);

void clean();
void sigint_fun(int signo);

////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    if(argc != 4 && argc != 5) {
        printf("EXPECTED ARGUMENTS: [NAME] [<UNIX> / <INET>] [ IPv4 ADDRESS, PORT NUMBER / UNIX PORT PATH ]\n");
        return 1;
    }

    signal(SIGINT, sigint_fun);

    name = argv[1];

    if(strcmp(argv[2], "UNIX") == 0)         sock_fd = get_unix_socket(argv[3]);
    else if(strcmp(argv[2], "INET") == 0)    sock_fd = get_inet_socket(atoi(argv[3]), argv[4]);
    else { printf("Error: unexpected argument: %s\n", argv[2]); return 1; }


    send_register_request();

    message_handling_loop();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////


int get_unix_socket(const char* socket_path) {
    int sock_fd;

    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1) { perror("get_unix_socket: socket method error"); exit(1); }

    sockaddr_un* sock_addr_bind = calloc(1, sizeof(sockaddr_un));
    sock_addr_bind->sun_family = AF_UNIX;
    strcpy(sock_addr_bind->sun_path, name);

    if (bind(sock_fd, (const struct sockaddr *) sock_addr_bind, sizeof(*sock_addr_bind))) {
        perror("get_unix_socket: bind method error");
        exit(1);
    }

    sockaddr_un* sock_addr_connect = calloc(1, sizeof(sockaddr_un));
    sock_addr_connect->sun_family = AF_UNIX;
    strcpy(sock_addr_connect->sun_path, socket_path);

    if (connect(sock_fd, (sockaddr*) sock_addr_connect, sizeof(*sock_addr_connect))) {
        perror("get_unix_socket: connect method error ");
        exit(1);
    }

    return sock_fd;
}



int get_inet_socket(int port_num,const char* IPv4_addr) {
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) { perror("get_inet_socket: socket method error"); exit(1); }

    sockaddr_in* sock_addr_connect = calloc(1, sizeof(sockaddr_in));
    sock_addr_connect->sin_family = AF_INET;
    sock_addr_connect->sin_addr.s_addr = inet_addr(IPv4_addr);
    sock_addr_connect->sin_port = htons(port_num);

    if (connect(sock_fd, (sockaddr*) sock_addr_connect, sizeof(*sock_addr_connect))) {
        perror("get_inet_socket: connect method error ");
        exit(1);
    }

    return sock_fd;
}



void send_register_request() {

    message* msg = calloc(1, sizeof(message));
    msg->message_type = REGISTER;
    strcpy(msg->client, name);

    pthread_mutex_lock(&mutex);
    if( send(sock_fd, msg, sizeof(message), 0) == -1 )
        perror("send_register_request: send REGISTER error");
    pthread_mutex_unlock(&mutex);

    uint8_t response;
    if (recv(sock_fd, &response, 1, 0) ==  -1) {
        printf("send_register_request: unexpected response from server");
        exit(1);
    }

    switch (response) {
        case SUCCESS:
            printf("Logged in successfully\n"); break;
        case FAILURE:
            printf("Error - connection unsuccessful"); break;
        default:
            printf("Error - send_register_request: unexpected response %d\n", response); break;
    }
}



void message_handling_loop() {

    pthread_t thread;
    message *request = calloc(1, sizeof(message));

    while (run) {

        if( recv(sock_fd, request, sizeof(message), 0) == -1 )       { if(run) perror("message_handling_loop: recv error"); exit(1); }

        switch (request->message_type) {

            case REQUEST:
                printf("Received request %d \n", request->ID);

                pthread_create(&thread, NULL, handle_message, request);
                pthread_detach(thread);
                break;

            case PING:
                pthread_mutex_lock(&mutex);
                printf(" -- PING --\n");

                message* msg = calloc(1, sizeof(message));
                msg->message_type = PING;
                strcpy(msg->client, name);

                if( send(sock_fd, msg, sizeof(message), 0) == -1 )
                    perror("message_handling_loop: send PING error");

                pthread_mutex_unlock(&mutex);
                break;

            case UNREGISTER:
                clean();

            default:
                printf("message_handling_loop: %d message type is not recognized\n", request->message_type);
                break;

        }
    }
}



void* handle_message(void * arg) {

    message* data = (message *)arg;

    // COUNTING WORDS
    char* command = calloc(MESSAGE_SIZE, sizeof(char));
    sprintf(command, "echo '%s' | tr '[:space:]' '[\\n*]' | grep -v \"^\\s*$\" | sort | uniq -c | sort -bnr", (char *) data->text);
    FILE* cmd_file = popen(command, "r");

    char* word_count = calloc(MESSAGE_SIZE, sizeof(char));
    fread (word_count, 1, MESSAGE_SIZE, cmd_file);

    memset(command, 0, MESSAGE_SIZE);
    pclose(cmd_file);

    // COUNTING WORDS NUMBER
    sprintf(command, "echo '%s' | wc -w", (char *) data->text);
    cmd_file = popen(command, "r");

    char* words_num = calloc(8, sizeof(char));
    fread (words_num, 8, MESSAGE_SIZE, cmd_file);

    memset(command, 0, MESSAGE_SIZE);
    pclose(cmd_file);

    // SAVING RESULT
    char* result = calloc(MESSAGE_SIZE, sizeof(char));
    sprintf(result, "id: %d     words: %s\n\n%s", data->ID, words_num, word_count);

    ///////////////////////////////////////////////////////////////////////////////

    // SENDIND
    pthread_mutex_lock(&mutex);

    message* result_msg = calloc(1, sizeof(message));
    result_msg->ID = data->ID;
    result_msg->message_type = RESULT;
    strcpy(result_msg->client, name);
    strcpy(result_msg->text, result);

    if( send(sock_fd, result_msg, sizeof(message), 0) == -1 )
        perror("handle_message: send error");

    printf("Result has been sent to server \n");

    pthread_mutex_unlock(&mutex);

    free(command);
    free(word_count);
    free(result);
    free(result_msg);

    return NULL;
}



void clean() {
    run = 0;

    if( close(sock_fd) == -1 )
        perror("Could not close Socket");

    unlink(name);
}



void sigint_fun(int signo) {

    message* msg = calloc(1, sizeof(message));
    msg->message_type = UNREGISTER;
    strcpy(msg->client, name);

    if( send(sock_fd, msg, sizeof(message), 0) == -1 )
        perror("clean: send UNREGISTER error");

    message *request = calloc(1, sizeof(message));

    if( recv(sock_fd, request, sizeof(message), 0) == -1 )  {
        perror("clean: recv error"); exit(1); }

    if( request->message_type == UNREGISTER ) printf("\n -- end --\n");

    clean();

}




