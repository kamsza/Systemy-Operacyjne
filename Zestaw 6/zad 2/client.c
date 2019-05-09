#include "client.h"
#include "message.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h> 
#include <unistd.h> 
#include <mqueue.h>

int client_id = -1;
mqd_t client_queue_id;
mqd_t server_queue_id;
char* client_name;


int main(int argc, char **argv) {
	// CREATE QUEUES
	signal(SIGINT, stop);
	
	client_name = calloc(256, 1);
	sprintf(client_name, "/new_client_%d", getpid());

	
	struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 256;
    attr.mq_curmsgs = 0;
	
	client_queue_id = mq_open(client_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);

	if(client_queue_id == -1) {
		perror("CLIENT: can not create queue - mq_open error");
		return 1;
	}


	server_queue_id = mq_open("/new_server", O_WRONLY);

	if(server_queue_id == -1) {
		perror("CLIENT: can not connect with server queue - mq_open error");
		return 1;
	}


	// SENDING INIT MESSAGE
	char* init_message = malloc(MESSAGE_SIZE);
	char* init_response = malloc(MESSAGE_SIZE);
	sprintf(init_message, "%d %s %s", -1, "INIT", client_name);

    if(mq_send(server_queue_id, init_message, MESSAGE_SIZE, INIT) == -1) {
		perror("CLIENT: error while sending INIT message to server");
		return 1;
	}

		
	
	if(mq_receive(client_queue_id, init_response, MESSAGE_SIZE, NULL) == -1) {
		perror("CLIENT: error while receiving INIT message from server");
		return 1;
	}

	
	char* id_str = strtok(init_response, " "); 
	client_id = atoi(id_str);
	
	if(client_id == -1) {
		printf("CLIENT: can not create client - server error\n");
		msgctl(client_queue_id, IPC_RMID, NULL);
		return 1;
	}
	
	printf("================================== CLIENT %d ==================================\n", client_id);

	// RUN
	char command[256];
	char to_send[256];

	pid_t pid = fork();
    if(pid == 0){
        while(running) {	
			// SENDIG TO SERVER
			if(fgets(command, MESSAGE_SIZE, stdin)) {
				sprintf(to_send, "%d %s", client_id, command);
				
				send_str(to_send);
			}
		}
    }
    else if(pid > 0) {
		// RECEIVING FROM SERVER
		char* string_received = malloc(MESSAGE_SIZE);
		
		while(running) {	
			if(mq_receive(client_queue_id, string_received, MESSAGE_SIZE, NULL) == -1) {
				perror("CLIENT: error while receiving message - mq_receive error");
				continue;
			}
			
			receive_str(string_received);	
		}
    }
    else {
        perror("CLIENT: fork error");
    }
    
    


	return 0;
}


// EXECUTING COMMANDS

void send_str(char* cmd) {
	
	struct message* msg = malloc(sizeof(struct message));
	msg = to_message(cmd);
	
	switch (msg->request_id) {
		case ADD: 
			if(msg->mtext == NULL) {
				printf("CLIENT: can not call ADD without arguments");
				return;
			}
			break;
        case DEL:
			if(msg->mtext == NULL) {
				printf("CLIENT: can not call DEL without arguments");
				return;
			}
			break;
			
		case READ:
			execute_cmd_from_file(msg->mtext);
			return;
		
		case ERROR:
			return;
			
        default:
            break;
    }
    
    send(cmd, msg->request_id);
   	
   	
    free(msg);
}


void send(char* cmd, int request_id) {
	if(mq_send(server_queue_id, cmd, MESSAGE_SIZE, request_id) == -1) {
		printf("CLIENT: error while sending %s message to server", cmd);
		perror("");
	}
}


void receive_str(char* cmd) {
	struct message* msg = malloc(sizeof(struct message));
	msg = to_message(cmd);

	switch (msg->request_id) {

        case STOP: 
			if(msgctl(client_queue_id, IPC_RMID, NULL) == -1)
				perror("CLIENT: can not close queue - msgctl error");
				
			running = 0;
			
			printf("CLIENT: stopped \n");
			break;
        default:
			printf(ANSI_COLOR_GREEN "RECEIVED: %s %s" ANSI_COLOR_RESET "\n", get_command(msg), msg->mtext);
            break;
            
    }
	free(msg);
}


void execute_cmd_from_file(char * path){
	FILE* file = fopen(path, "r");
	if(file == NULL) {
		char err[128];
		sprintf(err, "CLIENT: can not open file %s", path);
		perror(err);
    }
    
    char command[MESSAGE_SIZE];
    char to_send[MESSAGE_SIZE];
    
    while(fgets(command, MESSAGE_SIZE, file)) {
		sprintf(to_send,"%d %s", client_id, command);
			
		send_str(to_send);
	}
	
    fclose(file);
}

void stop() {
	char to_send[256];
	sprintf(to_send,"%d %s", client_id, "STOP");
	
	send_str(to_send);
}

