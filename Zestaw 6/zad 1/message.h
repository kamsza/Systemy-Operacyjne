#ifndef _MESSAGE_H
#define _MESSAGE_H

#define PROJ_ID 10
#define MESSAGE_SIZE 256

#define ANSI_COLOR_GREEN	"\x1b[32m"
#define ANSI_COLOR_BLUE		"\x1b[36m"
#define ANSI_COLOR_RESET	"\x1b[0m"

#define STOP 1
#define LIST 2
#define FRIENDS 3
#define INIT 4

#define ADD 11
#define DEL 12

#define ECHO 21
#define _2ONE 22
#define _2FRIENDS 23
#define _2ALL 24

#define READ 31

#define ERROR -1

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct message {
	long request_id;
	long client_id;
	char mtext[MESSAGE_SIZE]; 
};

struct message* to_message(char *command) {	
	char *cmd = strtok(command, " \n");
	char *args = strtok(NULL, "\n");
	
	struct message* msg = malloc(sizeof(struct message));
	msg->client_id = -1;
	
	if(args == NULL) strcpy(msg->mtext, "");
	else 	strcpy(msg->mtext, args);
	
	if( strcmp(cmd, "READ") == 0 ) 			msg->request_id = READ;
	
	else if( strcmp(cmd, "ECHO") == 0 )		msg->request_id = ECHO;
	else if( strcmp(cmd, "LIST") == 0 )	    msg->request_id = LIST;
	else if( strcmp(cmd, "FRIENDS") == 0 )  msg->request_id = FRIENDS;
	else if( strcmp(cmd, "2ALL") == 0 ) 	msg->request_id = _2ALL;
	else if( strcmp(cmd, "2FRIENDS") == 0 ) msg->request_id = _2FRIENDS;
	else if( strcmp(cmd, "2ONE") == 0 ) 	msg->request_id = _2ONE;
	else if( strcmp(cmd, "STOP") == 0 ) 	msg->request_id = STOP;
	
	else if( strcmp(cmd, "ADD") == 0 ) 		msg->request_id = ADD;
	else if( strcmp(cmd, "DEL") == 0 ) 		msg->request_id = DEL;
	
	else	{ printf("Command: %s not found\n", cmd); msg->request_id = ERROR; }
	
	return msg;
}

char* get_command(struct message* msg) {
	
	switch (msg->request_id) {
        case STOP: 		return "STOP";
        case LIST: 		return "LIST";
        case FRIENDS:	return "FRIENDS";
		case INIT: 		return "INIT";
        case ECHO: 		return "ECHO";
        case _2ALL:		return "2ALL";
        case _2FRIENDS: return "2FRIENDS";
        case _2ONE:		return "2ONE";
		case ADD: 		return "ADD";
        case DEL: 		return "DEL";			
        default:		return "Message not found";
    }
    
}


#endif
