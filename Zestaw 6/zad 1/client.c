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


int client_id = -1;
int client_queue_id;
int server_queue_id;



int main(int argc, char **argv) {
	// CREATE QUEUES
	signal(SIGINT, stop);
	
	client_queue_id = msgget(IPC_PRIVATE, QUEUE_PERMISSIONS);
	

	if(client_queue_id == -1) {
		perror("Can not create queue - msgget error \n");
		return 1;
	}

	char* home_dir = getenv("HOME");
	key_t queue_key = ftok(home_dir, PROJ_ID);
	server_queue_id = msgget(queue_key, 0);

	if(server_queue_id == -1) {
		perror("Can not connect with server queue - msgget error \n");
		return 1;
	}
	
	// SENDING INIT MESSAGE
	struct message init_message, init_response;
	init_message.request_id = INIT;
	init_message.client_id = -1;
	sprintf(init_message.mtext, "%d", client_queue_id);


    if(msgsnd(server_queue_id, &init_message, MESSAGE_SIZE, 0) == -1) {
		perror("Error while sending INIT message to server \n");
		return 1;
	}
	
	
	if(msgrcv(client_queue_id, &init_response, MESSAGE_SIZE, INIT,0) == -1) {
		perror("Error while receiving INIT message from server \n");
		return 1;
	}

	
	client_id = init_response.client_id;
	
	printf("================================== CLIENT %d ==================================\n", client_id);
	
	if(client_id == -1) {
		printf("Can not create client - server error \n");
		msgctl(client_queue_id, IPC_RMID, NULL);
		return 1;
	}
	else
		printf("CLIENT: created \n");

	// RUN
	char command[256];
	struct message* msg = malloc(sizeof(struct message));

	pid_t pid = fork();
    if(pid == 0){
        while(running) {	
			// SENDIG TO SERVER
			if(fgets(command, MESSAGE_SIZE, stdin)) {
			
				msg = to_message(command);
				msg->client_id = client_id;
			
				if(msg->request_id == ERROR) {
					printf(ANSI_COLOR_BLUE "CLIENT: received ERROR message" ANSI_COLOR_RESET "\n");
					continue;
				}
			
				send(msg);
			}
		}
    }
    else if(pid > 0) {
		// RECEIVING FROM SERVER
		struct message* message_received = malloc(sizeof(struct message));
		
		while(running) {	
			if(msgrcv(client_queue_id, message_received, MESSAGE_SIZE, -1000, 0) == -1) {
				perror("CLIENT: error while receiving message - msgrcf error \n");
				continue;
			}
			
			receive(message_received);	
		}
    }
    else {
        perror("CLIENT: fork error \n");
    }
    
    


	return 0;
}


// EXECUTING COMMANDS

void send(struct message* msg) {
	
	switch (msg->request_id) {
		case ADD: 
			if(strcmp(msg->mtext, "") == 0) {
				printf("Can not call ADD without arguments \n");
				return;
			}
			break;
        case DEL:
			if(strcmp(msg->mtext, "") == 0) {
				printf("Can not call DEL without arguments \n");
				return;
			}
			break;
			
		case READ:
			execute_cmd_from_file(msg->mtext);
			return;
	
        default:
            break;
    }
    
    if(msgsnd(server_queue_id, msg, MESSAGE_SIZE, 0) == -1) 
		printf("Error while sending %s message to server \n", get_command(msg));
}


void receive(struct message* msg) {
	
	switch (msg->request_id) {

        case STOP: 
			if(msgctl(client_queue_id, IPC_RMID, NULL) == -1)
				perror("Can not close queue - msgctl error \n");
			else	
				running = 0;
			break;
        default:
			printf(ANSI_COLOR_GREEN "RECEIVED: %s %s" ANSI_COLOR_RESET "\n", get_command(msg), msg->mtext);
            break;
    }
	
	
}


void execute_cmd_from_file(char * path){
	FILE* file = fopen(path, "r");
	if(file == NULL) {
		char err[128];
		sprintf(err, "Can not open file %s \n", path);
		perror(err);
    }
    
    char command[MESSAGE_SIZE];
    struct message* msg = malloc(sizeof(struct message));
    
    while(fgets(command, MESSAGE_SIZE, file)) {
		msg = to_message(command);
		if(msg->request_id == ERROR) {
			printf(ANSI_COLOR_BLUE "CLIENT: received ERROR message" ANSI_COLOR_RESET "\n");
			continue;
		}
		
		msg->client_id = client_id;
			
		send(msg);
	}
	
    fclose(file);
}

void stop() {
	struct message* msg = malloc(sizeof(struct message));
	msg->client_id = client_id;
	msg->request_id = STOP;
	strcpy(msg->mtext, "");
	
	send(msg);
}

