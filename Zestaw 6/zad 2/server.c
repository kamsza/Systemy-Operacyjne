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
#include <mqueue.h>

int client_queue_id[MAX_NUM_OF_CLIENTS]; 
char friend_list[MAX_NUM_OF_CLIENTS][MAX_NUM_OF_CLIENTS];		 // truth table
char* server_name = "/new_server";

int main(int argc, char **argv)
{
	signal(SIGINT, handleSIGINT);
	
	// CREATE
	init_client_list();
	
	mqd_t queue_id;
	struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 256;
    attr.mq_curmsgs = 0;
	
	queue_id = mq_open(server_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);
	
	if(queue_id == -1) {
		perror("SERVER: can not create queue - mq_open error");
		return 1;
	}
	
	printf("=================================== SERVER ===================================\n");

	char string_received[MESSAGE_SIZE];
	struct message* message_received = malloc(sizeof(struct message));

	// RUN
	while(running) {	
		if(mq_receive(queue_id, string_received, MESSAGE_SIZE, NULL) == -1) {
			perror("SERVER: error while receiving message - mq_receive error");
			continue;
		}
		
		message_received = to_message(string_received);
		
		printf(ANSI_COLOR_GREEN "RECEIVED: %s %s  from %ld" ANSI_COLOR_RESET "\n", get_command(message_received), message_received->mtext, message_received->client_id);
		
		execute_cmd( message_received );
	}
	
	// END
    if (mq_close(queue_id) == -1) {
        perror("SERVER: can not close queue - mq_close error");
        return 1;
    }

    if (mq_unlink(server_name) == -1) {
        printf("SERVER: error while remove server queue.\n");
    }
    
     printf("SERVER: stopped\n");

    return 0;
}


// COMMANDS EXECUTION

void execute_cmd(struct message* received) {
		
	if((client_queue_id[received->client_id] == -1) && (received->request_id != INIT)) {
		printf("execute_cmd failed - client %ld doesn't exist\n", received->client_id);
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
	send(received);
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
	
	send(to_send);
}


void init(struct message* received) {
	struct message* to_send = malloc(sizeof(struct message));
	
	char* client_name = received->mtext;
	int new_client_queue_id = mq_open(client_name, O_WRONLY);

	if(new_client_queue_id == -1) {
		perror("SERVER: INIT failed - mq_open error");
		return;
	}
	


	if(check_client(new_client_queue_id)) {
		printf("SERVER: INIT failed - client already exist\n");
		return;
	}

	int new_client_id = get_free_index();

	
	if(new_client_id == -1) {
		printf("SERVER: INIT failed - client_queue_id table full\n");
		return;
	}
	

	client_queue_id[new_client_id] = new_client_queue_id;
	to_send->client_id = new_client_id;
	to_send->request_id = INIT;
	strcpy(to_send->mtext, "");
	
	
	send(to_send);
}

void to_all(struct message* received) {
	for(int c = 0; c < MAX_NUM_OF_CLIENTS; c++) {
		if(client_queue_id[c] != -1) {
			send_to(client_queue_id[c], received);
		}
	}
}

void to_one(struct message* received) {
	char* id = strtok(received->mtext, " ");
	char* args = strtok(NULL, "\n");
	int id_i = atoi(id);
	
	strcpy(received->mtext, args);
	
	printf("--%d--%s--\n", id_i, received->mtext);
	
	send_to(client_queue_id[id_i], received);
}

void to_friends(struct message* received) {
	for(int c = 0; c < MAX_NUM_OF_CLIENTS; c++) {
		if(friend_list[received->client_id][c]) {
			send_to(client_queue_id[c], received);
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

void send(struct message* msg) {
	char* command = to_string(msg);
	
	
	if(mq_send(client_queue_id[msg->client_id], command, MESSAGE_SIZE, msg->request_id) == -1) 
		printf("Error while sending message to server\n");
	else 
		printf(ANSI_COLOR_BLUE "SENT:     %s %s to %ld" ANSI_COLOR_RESET "\n", get_command(msg), msg->mtext, msg->client_id);
}

void send_to(int client_queue_id, struct message* msg) {
	char* command = to_string(msg);
	
	
	if(mq_send(client_queue_id, command, MESSAGE_SIZE, msg->request_id) == -1) 
		printf("Error while sending message to server\n");
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
			send_to(client_queue_id[c], msg);
		}
	}
	
	running = 0;
	printf("\n\nSERVER: stopped \n");
}


