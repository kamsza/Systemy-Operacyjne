#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>




const int MAX_LINE_LENGTH = 128;
int MONITORED_ELEMENTS;
int running = 1;
int ended = 0;

void stop(int signum) {
    running = 0;
}

void start(int signum) {
    running = 1;
}

void end(int sig) {
    ended = 1;
}


monitor* load_list(char* list) {
    FILE *fp = fopen(list, "r");
    if(fp == NULL) {
        printf("Error opening %s file\n", list);
        return NULL;
    }

    char ch;
    int list_size = 0;
    while((ch = fgetc(fp)) !=  EOF){
        if(ch == '\n'){
            list_size ++;
        }}

    fseek(fp, 0L, SEEK_SET);
    MONITORED_ELEMENTS = list_size;

    monitor* new_monitor_list = calloc(list_size, sizeof(monitor));
    int index = 0;

    char* curr_line = calloc(MAX_LINE_LENGTH, sizeof(char));
    char* buff;
    while(fgets(curr_line, MAX_LINE_LENGTH, fp)) {
        buff = strtok(curr_line, " ");
        new_monitor_list[index].file_name = malloc(sizeof(buff));
        strcpy(new_monitor_list[index].file_name, buff);

        buff = strtok(NULL, " ");
        new_monitor_list[index].file_path = malloc(sizeof(buff));
        strcpy(new_monitor_list[index].file_path, buff);

        buff = strtok(NULL, " ");
        new_monitor_list[index].monitoring_freq_time = atoi(buff);
        new_monitor_list[index].modification_time = -1;
        new_monitor_list[index].content = NULL;
        new_monitor_list[index].pid = -2;
		  new_monitor_list[index].state = 0;

        buff = strtok(NULL, " ");
        if(buff != NULL) {printf(" %s \n", buff); return NULL;}

        index++;
    }

    fclose(fp);
    return new_monitor_list;
}





void archive_content(monitor *m) {
    DIR* dir = opendir("archive");
    if(dir == NULL) {
        mkdir("archive", 0777);
    }

    char modification_time[128];
    strftime(modification_time, 128, "_%Y-%m-%d_%H:%M:%S", localtime(&m->modification_time));
    char* file_name = calloc(128, sizeof(char));
    strcat(file_name, "archive/");
    strcat(file_name, m->file_name);
    strcat(file_name, modification_time);

    FILE* fp = fopen(file_name, "w");
    if(fp == NULL) printf("== Couldn't create new file: %s ==\n", file_name);
    else {
        int res = fprintf(fp, "%s", m->content);
        if(res < 0){printf("== Writing content of %s file to file: %s unsuccessful ==\n", m->file_name, file_name);}
        fclose(fp);
    }

    closedir(dir);
}


void load_new_content(monitor *m) {
    FILE* fp = fopen(m->file_path, "r");
    if(fp == NULL){
        printf("== Couldn't open file: %s == \n", m->file_path);
        return;
    }
    struct stat file_stat;
    if(lstat(m->file_path, &file_stat) < 0) {
        printf("== Couldn't read stats of %s file ==\n", m->file_path);
        return;
    }
    m->content = malloc(file_stat.st_size + 1);
    long read = fread(m->content, sizeof(char), file_stat.st_size, fp);
    if(read != file_stat.st_size) {
        printf("Only %li bytes read od %li bytes in file %s \n", read, file_stat.st_size, m->file_name);
    }
    m->content[file_stat.st_size] = '\0';
    fclose(fp);
}









void monitoring(monitor *m) {
    signal(SIGUSR1, &stop);
    signal(SIGUSR2, &start);
    signal(SIGRTMIN, &end);

    int copies_created = 0;
    struct stat file_stat;

    if(lstat(m->file_path, &file_stat) < 0) {
        printf("== Couldn't read stats of %s file ==\n", m->file_path);
        exit(1);
    }
    m->modification_time = file_stat.st_mtime;

    while(!ended) {
        while (running) {
            if (lstat(m->file_path, &file_stat) < 0) {
                printf("== Couldn't read stats of %s file ==\n", m->file_path);
                exit(1);
            }

            if (file_stat.st_mtime != m->modification_time) {
                copies_created++;
                if (m->content != NULL) archive_content(m);
                load_new_content(m);
                m->modification_time = file_stat.st_mtime;
            }
            sleep((unsigned int) m->monitoring_freq_time);



            if(ended)  break;
        }
        if(ended) break;
        if(!running) sleep(1);
    }

    exit(copies_created);
}




void start_monitoring(monitor *m) {
    for (int i = 0; i < MONITORED_ELEMENTS; i++) {
        pid_t pid = fork();

        if (pid == 0)
            monitoring(&m[i]);
        else
            m[i].pid = pid;
				m[i].state = 1;
    }

    while(1) {
        char* command = calloc(16, sizeof(char));
        scanf("%s", command);

        if(strcmp(command, "LIST") == 0) {
            for(int i = 0; i < MONITORED_ELEMENTS; i++)
                printf("file: %s  pid: %d  state: %s \n", m[i].file_name, m[i].pid, m[i].state ? "running" : "stopped");
        }

        else if(strcmp(command, "STOP") == 0){
				char* pid_str = calloc(16, sizeof(char));
				scanf("%s", pid_str);

				if(strcmp(pid_str, "ALL") == 0) {
            	for(int i = 0; i < MONITORED_ELEMENTS; i++) {
               	kill(m[i].pid, SIGUSR1);
						m[i].state = 0;
               	running = 0;
            	}
				}
				
				else {
            	int pid = atoi(pid_str);

            	for(int i = 0; i < MONITORED_ELEMENTS; i++) {
                	if(m[i].pid == pid) {
                    	kill(pid, SIGUSR1);
                    	running = 0;
						   m[i].state = 0;
                    	pid = -1;
                   	 break;
                	}
            	}
            	if(pid > 0) {
                	printf("== Proces with pid %s not found == \n", pid_str);
            	}
				}
        }

        else if(strcmp(command, "START") == 0){
				char* pid_str = calloc(16, sizeof(char));
				scanf("%s", pid_str);

				if(strcmp(pid_str, "ALL") == 0) {
            	for(int i = 0; i < MONITORED_ELEMENTS; i++) {
               	kill(m[i].pid, SIGUSR2);
               	running = 1;
            	}
				}
	
				else {
					int pid = atoi(pid_str);

            	for(int i = 0; i < MONITORED_ELEMENTS; i++) {
                	if(m[i].pid == pid) {
                    	kill(pid, SIGUSR2);
                    	running = 1;
                    	pid = -1;
                    	break;
                	}
            	}
            	if(pid > 0) {
               	 printf("== Proces with pid: %s not found == \n", pid_str);
            	}
				}
        }

        else if(strcmp(command, "END") == 0) {
            for(int i = 0; i < MONITORED_ELEMENTS; i++) {
                int res;
                kill(m[i].pid, SIGTERM);
                waitpid(m[i].pid, &res, 0);
                printf("file: %s  pid: %d backups: %d \n", m[i].file_name, m[i].pid, WEXITSTATUS(res));
            }
				exit(0);
        }

        else {printf("== Command ' %s ' not found. == \n", command);}

        free(command);
    }
}



























