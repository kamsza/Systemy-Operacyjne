#ifndef _CLIENT_H
#define _CLIENT_H

#define QUEUE_PERMISSIONS 0666

#include "message.h"

int running = 1;

void execute_cmd_from_file(char *);
void stop();

void send(struct message*);
void receive(struct message* msg);


#endif
