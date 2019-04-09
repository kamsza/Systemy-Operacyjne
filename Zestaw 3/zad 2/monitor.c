#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>




void show_monitor(monitor m) {
    printf("File name: %s \n", m.file_name);
    printf("File path: %s \n", m.file_path);
    printf("Monitoring time: %d \n", m.monitoring_time);
    printf("Monitoring freq: %d \n", m.monitoring_freq_time);
}



const int MAX_LINE_LENGTH = 128;
int MONITORED_ELEMENTS;





monitor* load_list(char* list, char* monitoring_time) {
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
        new_monitor_list[index].monitoring_time = atoi(monitoring_time);
        new_monitor_list[index].modification_time = -1;
        new_monitor_list[index].content = NULL;

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
    if(fp == NULL) printf("== Couldn't create new file: %s ==", file_name);
    else {
        int res = fprintf(fp, "%s", m->content);
        if(res < 0){printf("== Writing content of %s file to file: %s unsuccessful ==", m->file_name, file_name);}
        fclose(fp);
    }

    closedir(dir);
}


void load_new_content(monitor *m) {
    FILE* fp = fopen(m->file_path, "r");
    if(fp == NULL){
        printf("== Couldn't open file: %s ==", m->file_path);
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


void start_monitoring_l(monitor *m) {
    int copies_created = 0;
    int time = 0;
    int end_time = m->monitoring_time;
    struct stat file_stat;

    if(lstat(m->file_path, &file_stat) < 0) {
        printf("== Couldn't read stats of %s file ==\n", m->file_path);
        exit(1);
    }
    m->modification_time = file_stat.st_mtime;

    while(time <= end_time) {
        if(lstat(m->file_path, &file_stat) < 0) {
            printf("== Couldn't read stats of %s file ==\n", m->file_path);
            exit(1);
        }

        if(file_stat.st_mtime != m->modification_time) {
            copies_created++;
            if(m->content != NULL) archive_content(m);
            load_new_content(m);
            m->modification_time = file_stat.st_mtime;
        }
        sleep((unsigned int)m->monitoring_freq_time);
        time += (m->monitoring_freq_time);
    }

    exit(copies_created);
}









void copy_file(monitor *m) {
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

    pid_t pid = fork();
    if(pid == 0) {
        execlp("cp", "cp", m->file_path, file_name, NULL);

    }
    else {
        int status;
        wait(&status);
        if(status != 0) {printf("== copyfile for %s was not successful ==", m->file_name);}
    }
	closedir(dir);
}


void start_monitoring_cp(monitor *m) {
    int copies_created = 0;
    int time = 0;
    int end_time = m->monitoring_time;
    struct stat file_stat;

    if(lstat(m->file_path, &file_stat) < 0) {
        printf("== Couldn't read stats of %s file ==\n", m->file_path);
        exit(1);
    }
    m->modification_time = file_stat.st_mtime;

    while(time <= end_time) {
        if(lstat(m->file_path, &file_stat) < 0) {
            printf("== Couldn't read stats of %s file ==\n", m->file_path);
            exit(1);
        }

        if(file_stat.st_mtime != m->modification_time) {
            copy_file(m);
            copies_created++;
            m->modification_time = file_stat.st_mtime;
        }

        sleep((unsigned int)m->monitoring_freq_time);
        time += (m->monitoring_freq_time);
    }

    exit(copies_created);
}








void start_monitoring(monitor *m, enum mode mode) {
      for (int i = 0; i < MONITORED_ELEMENTS; i++) {
          pid_t pid = fork();

          if (pid == 0) {
              if (mode == load) start_monitoring_l(&m[i]);
              else if (mode == copy) start_monitoring_cp(&m[i]);
              else printf("Unknown mode value");
          }
      }


      int res;
      for (int i = 0; i < MONITORED_ELEMENTS; i++) {
          pid_t curr = wait(&res);
          printf("Process %d created %d file copies.\n", curr, WEXITSTATUS(res));
    }
}








