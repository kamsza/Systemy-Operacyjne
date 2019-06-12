#include "message.h"

char *name;
int sock_fd;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



int get_unix_socket(const char*);
int get_inet_socket(int, const char*);

void send_register_request();
void send_message(uint8_t);

void message_handling_loop();
void* handle_message(void * arg);

void clean();

//////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    if(argc != 4 && argc != 5) {
        printf("EXPECTED ARGUMENTS: [NAME] [<UNIX> / <INET>] [ IPv4 ADDRESS, PORT NUMBER / UNIX PORT PATH ]\n");
        return 1;
    }

    signal(SIGINT, clean);
    atexit(clean);

    name = argv[1];

    if(strcmp(argv[2], "UNIX") == 0)         sock_fd = get_unix_socket(argv[3]);
    else if(strcmp(argv[2], "INET") == 0)    sock_fd = get_inet_socket(atoi(argv[3]), argv[4]);
    else { printf("Error: unexpected argument: %s\n", argv[2]); return 1; }


    send_register_request();

    message_handling_loop();

    return 0;
}

//////////////////////////////////////////////////////////////


int get_unix_socket(const char* socket_path) {
    int sock_fd;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) { perror("get_unix_socket: socket method error"); exit(1); }

    sockaddr_un* server_addr = calloc(1, sizeof(server_addr));
    server_addr->sun_family = AF_UNIX;
    strcpy(server_addr->sun_path, socket_path);

    if (connect(sock_fd, (sockaddr*) server_addr, sizeof(*server_addr))) {
        perror("get_unix_socket: connect method error ");
        exit(1);
    }

    return sock_fd;
}



int get_inet_socket(int port_num,const char* IPv4_addr) {
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_STREAM,0);
    if (sock_fd == -1) { perror("get_inet_socket: socket method error"); exit(1); }

    sockaddr_in* server_addr = calloc(1, sizeof(server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr(IPv4_addr);
    server_addr->sin_port = htons(port_num);

    if (connect(sock_fd, (sockaddr*) server_addr, sizeof(*server_addr))) {
        perror("get_inet_socket: connect method error ");
        exit(1);
    }

    return sock_fd;
}



void send_register_request() {
    send_message(REGISTER);

    int response;
    if (recv(sock_fd, &response, 1, 0) != 1) {
        printf("send_register_request: unexpected response from server");
        exit(1);
    }

    switch (response) {
        case SUCCESS:
            printf("Logged in successfully\n"); break;
        case FAILURE:
            printf("Error - connection unsuccessful"); break;
    }
}



void send_message(uint8_t message_type) {
    uint16_t message_size = (uint16_t) (strlen(name) + 1);

    send(sock_fd, &message_type, TYPE_SIZE, 0);
    send(sock_fd, &message_size, LEN_SIZE, 0);
    send(sock_fd, name, message_size, 0);
}



void message_handling_loop() {

    uint8_t message_type;
    pthread_t thread;
    message *request;


    while (1) {

        if( recv(sock_fd, &message_type, TYPE_SIZE, 0) == -1 )       { perror("message_handling_loop: recv message_type error"); exit(1); }

        switch (message_type) {
            case REQUEST:
                request = (message*) calloc(1,sizeof(message));
                uint16_t req_len;

                if( recv(sock_fd, &req_len, 2, 0) == -1 )            { perror("message_handling_loop: recv req_len error"); exit(1); }
                if( recv(sock_fd, request->text, req_len, 0) == -1 ) { perror("message_handling_loop: recv text error"); exit(1); }

                printf("Received request %d \n", request->ID);

                pthread_create(&thread, NULL, handle_message, request);
                pthread_detach(thread);
                break;

            case PING:
                pthread_mutex_lock(&mutex);
                send_message(PING);
                pthread_mutex_unlock(&mutex);
                break;

            default:
                printf("message_handling_loop: %d message type is not recognized\n", message_type);
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


    pthread_mutex_lock(&mutex);

    send_message(RESULT);

    int len = strlen(result);
    if (send(sock_fd, &len, sizeof(int), 0) == -1) { perror("handle_message: send len error"); exit(1); }
    if (send(sock_fd, result, len, 0) == -1)    { perror("handle_message: send result error"); exit(1); }

    printf("Result has been sent to server \n");

    pthread_mutex_unlock(&mutex);

    free(command);
    free(word_count);
    free(result);

    return NULL;
}



void clean() {
    send_message(UNREGISTER);
    if (shutdown(sock_fd, SHUT_RDWR) == -1)
        perror("Could not shutdown Socket");
    if (close(sock_fd) == -1)
        perror("Could not close Socket");
}
