#ifndef _SERVER_H
#define _SERVER_H

#define QUEUE_PERMISSIONS 0666

#define MAX_NUM_OF_CLIENTS 10

#include "message.h"

int running = 1;

void handleSIGINT(int sig_num);

void init_client_list();
int get_free_index();
int check_client(int queue_id);

void execute_cmd(struct message*);
void send(struct message*);
void send_to(int, struct message*);

void init(struct message*);
void stop(struct message*);
void list();
void friends(struct message*);
void echo(struct message*);

void to_all(struct message* received);
void to_one(struct message* received);
void to_friends(struct message* received);

void add_friend(struct message* received);
void del_friend(struct message* received);

char* get_time();
#endif

