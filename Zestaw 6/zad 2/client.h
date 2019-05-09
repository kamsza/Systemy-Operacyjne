#ifndef _CLIENT_H
#define _CLIENT_H

#define QUEUE_PERMISSIONS 0666

#include "message.h"

int running = 1;

void execute_cmd_from_file(char *);
void stop();

void send_str(char* cmd);
void send(char* cmd, int request_id);
void receive_str(char* cmd);


#endif
