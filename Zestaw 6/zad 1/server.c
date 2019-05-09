#include "server.h"
#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h> 
#include <signal.h> 

int client_queue_id[MAX_NUM_OF_CLIENTS]; 
char friend_list[MAX_NUM_OF_CLIENTS][MAX_NUM_OF_CLIENTS];		 // truth table

int main(int argc, char **argv)
{
	signal(SIGINT, handleSIGINT);
	
	// CREATE
	printf("=================================== SERVER ===================================\n");

	init_client_list();
	char* home_dir = getenv("HOME");
	key_t queue_key = ftok(home_dir, PROJ_ID);
	
	if(queue_key == -1) {
		perror("SERVER: can not get unique key - ftok error \n");
		return 1;
	}
	
	int queue_id = msgget(queue_key, IPC_CREAT | QUEUE_PERMISSIONS);
	
	if(queue_id == -1) {
		perror("SERVER: can not create queue - msgget error \n");
		return 1;
	}
	
	printf("SERVER: server is running\n");

	struct message* message_received = malloc(sizeof(struct message));
	
	// RUN
	while(running) {	
		
		if(msgrcv(queue_id, message_received, MESSAGE_SIZE, -1000, 0) == -1) {
			perror("SERVER: error while receiving message - msgrcf error \n");
			continue;
		}
		
		printf(ANSI_COLOR_GREEN "RECEIVED: %s %s  from %ld" ANSI_COLOR_RESET "\n", get_command(message_received), message_received->mtext, message_received->client_id);
		
		execute_cmd(message_received);
	}
	
	
	// END
    if(msgctl(queue_id, IPC_RMID, NULL) == -1) {
        perror("SERVER: can not close queue - msgctl error \n");
        return 1;
    }

    return 0;
}


// COMMANDS EXECUTION

void execute_cmd(struct message* received) {
		
	if((client_queue_id[received->client_id] == -1) && (received->request_id != INIT)) {
		printf("SERVER: execute_cmd failed - client %ld doesn't exist \n", received->client_id);
		return;
	}
	
	switch (received->request_id) {
        case STOP: 
			stop(received);
			break;
        case LIST: 
			list();
			break;
        case FRIENDS:
			friends(received);
			break;
		case INIT: 
			init(received);
			break;
        case ECHO: 
			echo(received);
			break;
        case _2ALL: 
			to_all(received);
			break;
        case _2FRIENDS: 
			to_friends(received);
			break;
        case _2ONE:
			to_one(received);
			break;
		
		case ADD: 
			add_friend(received);
			break;
        case DEL:
			del_friend(received);
			break;
			
        default:
            break;
    }
}


void stop(struct message* received) {
	send(client_queue_id[received->client_id], received);
	client_queue_id[received->client_id] = -1;
	for(int c = 0;  c < MAX_NUM_OF_CLIENTS; c++)
		friend_list[c][received->client_id] = 0;
}



void list() {
	printf("\n------------------------------------\n");
	printf("            CLIENTS LIST\n");
	for(int c = 0; c < MAX_NUM_OF_CLIENTS; c++)
		if(client_queue_id[c] != -1)
			printf("client id: %d, queue id: %d \n", c, client_queue_id[c]);
	printf("------------------------------------\n\n");
}

void friends(struct message* received) {
	for(int c = 0; c < MAX_NUM_OF_CLIENTS; c++) 
		friend_list[received->client_id][c] = 0;
	
	if(strcmp(received->mtext, "") == 0) 
		return;

	char* friend_id = strtok(received->mtext, " ");
	while(friend_id != NULL) {
		friend_list[received->client_id][atoi(friend_id)] = 1;
		friend_id = strtok(NULL, " ");
	}
} 

void echo(struct message* received) {
	struct message* to_send = malloc(sizeof(struct message));
	to_send->client_id = received->client_id;
	to_send->request_id = ECHO;
	strcpy(to_send->mtext, get_time());
	strcat(to_send->mtext, received->mtext);
	
	send(client_queue_id[to_send->client_id], to_send);
}


void init(struct message* received) {
	struct message* to_send = malloc(sizeof(struct message));
	
	int new_client_queue_id = atoi(received->mtext);
	
	if(check_client(new_client_queue_id)) {
		printf("SERVER: INIT failed - client already exist \n");
		return;
	}
	
	int new_client_id = get_free_index();
	
	
	
	if(new_client_id == -1) {
		printf("SERVER: INIT failed - client_queue_id table full \n");
		return;
	}
	

	client_queue_id[new_client_id] = new_client_queue_id;
	to_send->client_id = new_client_id;
	to_send->request_id = INIT;
	strcpy(to_send->mtext, "");
	
	
	send(new_client_queue_id, to_send);
}

void to_all(struct message* received) {
	for(int c = 0; c < MAX_NUM_OF_CLIENTS; c++) {
		if(client_queue_id[c] != -1) {
			send(client_queue_id[c], received);
		}
	}
}

void to_one(struct message* received) {
	char* id = strtok(received->mtext, " ");
	char* args = strtok(NULL, "\n");
	int id_i = atoi(id);
	
	strcpy(received->mtext, args);
	
	printf("--%d--%s--\n", id_i, received->mtext);
	
	send(client_queue_id[id_i], received);
}

void to_friends(struct message* received) {
	for(int c = 0; c < MAX_NUM_OF_CLIENTS; c++) {
		if(friend_list[received->client_id][c]) {
			send(client_queue_id[c], received);
		}
	}
}

void add_friend(struct message* received) {
	char *new_friend = strtok(received->mtext, " \n");
	int id;
	
	while(new_friend != NULL) {
		id = atoi(new_friend);
		
		if(id < 0 || id >= MAX_NUM_OF_CLIENTS) 
			printf("can not add %d client to friend list - wrong id \n", id);
		else
			friend_list[received->client_id][id] = 1;
			
		new_friend = strtok(NULL, " \n");
	}
}

void del_friend(struct message* received) {
	char *not_friend = strtok(received->mtext, " \n");
	int id;
	
	while(not_friend != NULL) {
		id = atoi(not_friend);
		if(id < 0 || id >= MAX_NUM_OF_CLIENTS) 
			printf("can not add %d client to friend list - wrong id \n", id);
		else
			friend_list[received->client_id][id] = 0;
		not_friend = strtok(NULL, " \n");
	}
}

// CLIENT LIST HANDLING

void init_client_list() {
	for(int c = 0; c < MAX_NUM_OF_CLIENTS; c++) {
		client_queue_id[c] = -1;
		
		for(int g = 0; g < MAX_NUM_OF_CLIENTS; g++)
		friend_list[c][g] = 0;
	}
}

int get_free_index() {
	for(int c = 0; c < MAX_NUM_OF_CLIENTS; c++) 
		if(client_queue_id[c] == -1) return c;
	
	return -1;
}

int check_client(int queue_id) {
	for(int c = 0; c < MAX_NUM_OF_CLIENTS; c++)
		if(client_queue_id[c] == queue_id) return 1;
	return 0;
}

// SENDING MESSAGES

void send(int client_queue_id, struct message* msg) {
	if(msgsnd(client_queue_id, msg, MESSAGE_SIZE, 0) == -1) 
		printf("Error while sending message to server \n");
	else 
		printf(ANSI_COLOR_BLUE "SENT:     %s %s to %ld" ANSI_COLOR_RESET "\n", get_command(msg), msg->mtext, msg->client_id);
}

// OTHERS

char* get_time() {
	time_t rawtime;
	time( &rawtime );
	struct tm *time = localtime( &rawtime );
	char* str_time = calloc(21, sizeof(char));
	strftime(str_time, 21,"%d-%m-%Y %H:%M:%S ", time);
	return str_time;
}




void handleSIGINT(int sig_num) {
	struct message* msg = malloc(sizeof(struct message));
	msg->request_id = STOP;
	strcpy(msg->mtext, "");
	
	for(int c = 0; c < MAX_NUM_OF_CLIENTS; c++) {
		if(client_queue_id[c] != -1) {
			msg->client_id = c;
			send(client_queue_id[c], msg);
		}
	}
	
	running = 0;
	printf("\n\nSERVER: stopped \n");
}


