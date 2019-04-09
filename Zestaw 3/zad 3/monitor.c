#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>



void show_monitor(monitor m) {
    printf("File name: %s \n", m.file_name);
    printf("File path: %s \n", m.file_path);
    printf("Monitoring time: %d \n", m.monitoring_time);
    printf("Monitoring freq: %d \n", m.monitoring_freq_time);
}



const int MAX_LINE_LENGTH = 256;
int MONITORED_ELEMENTS = -1;



monitor* load_list(char* list, char* monitoring_time, int max_cpu_t, int max_memory) {
    FILE *fp = fopen(list, "r");
    if(fp == NULL) {
        printf("== Error opening %s file in load_list() function==\n", list);
        return NULL;
    }

    char ch;
    int list_size = 0;
    while((ch = fgetc(fp)) !=  EOF){if(ch == '\n'){list_size ++;}}

	if(list_size == 0) {
		  printf("== Can not create monitor fo empty file: %s ==\n", list);
        return NULL;
    }

    fseek(fp, 0L, SEEK_SET);
    MONITORED_ELEMENTS = list_size;

    monitor* new_monitor_list = calloc(list_size, sizeof(monitor));
	 if(new_monitor_list == NULL) {
       printf("== calloc returned NULL pointer in load_list() to new_monitor_list ==\n");
       fclose(fp);
       return NULL;
	 }
    int index = 0;

    char* curr_line = calloc(MAX_LINE_LENGTH, sizeof(char));
	 if(curr_line == NULL) {
       printf("== calloc returned NULL pointer in load_list() to curr_line ==\n");
		 free(new_monitor_list);
       fclose(fp);
       return NULL;
	 }
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

		  new_monitor_list[index].max_cpu_t = max_cpu_t;
		  new_monitor_list[index].max_memory = max_memory;

        buff = strtok(NULL, " ");
        if(buff != NULL) {
					printf("== Unexpected characters ' %s ' at the end of the line number %d == \n", buff, index); 
					return NULL;
			}
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

    char modification_time[32];
    strftime(modification_time, 32, "_%Y-%m-%d_%H:%M:%S", localtime(&m->modification_time));
    char* file_name = calloc(sizeof(m->file_name) + sizeof("archive/") + sizeof(modification_time), sizeof(char));
	 if(file_name == NULL) {
			if(errno ==  ENOMEM) {printf("== archive_content() out of memory ==\n"); exit(255);}
         else printf("== calloc returned NULL pointer in archive_content() ==\n");
	 }
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


    m->content = calloc(file_stat.st_size + 1, sizeof(char));
    if(m->content == NULL) {
			if(errno ==  ENOMEM) {printf("== load_new_content() out of memory ==\n"); exit(255);}
         else printf("== calloc returned NULL pointer in load_new_content() ==\n");
	 }
    
    long read = fread(m->content, sizeof(char), file_stat.st_size, fp);
    if(read != file_stat.st_size) {
        printf("== Only %ld bits read of %ld bits in file %s ==\n", read, file_stat.st_size, m->file_name);
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
    load_new_content(m);

    while(time <= end_time) {
        if(lstat(m->file_path, &file_stat) < 0) {
            printf("== Couldn't read stats of %s file ==\n", m->file_path);
            exit(1);
        }

        if(file_stat.st_mtime != m->modification_time) {
            copies_created++;
            archive_content(m);
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

    char modification_time[32];
    int res = strftime(modification_time, 32, "_%Y-%m-%d_%H:%M:%S", localtime(&m->modification_time));
    if(res != 0) {
        printf("== Only %d bits read by strftime in copy_file() ==\n", res);
    }

    char* file_name = calloc(sizeof(m->file_name) + sizeof("archive/") + sizeof(modification_time), sizeof(char));
	 if(m->content == NULL) {
			if(errno ==  ENOMEM) {printf("== acopy_file() out of memory ==\n"); exit(255);}
         else {printf("== file_name = NULL in copy_file() =="); return;}
	 }
    strcat(file_name, "archive/");
    strcat(file_name, m->file_name);
    strcat(file_name, modification_time);

    pid_t pid = fork();
    if(pid == 0) {
        execlp("cp", "cp", m->file_path, file_name, NULL);
		  perror("== exec(cp) in copy_file() failure == \n"); 
        exit(1); 
    }
    else {
        int status;
        wait(&status);
        if(WEXITSTATUS(status)) {printf("== Copyfile for %s was not successful. Exit with code = %d == \n", m->file_name, WEXITSTATUS(status));}
    }
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


void sayHello(int signum) {
	printf("Hello");
}



void start_monitoring(monitor *m, enum mode mode) {
     signal(SIGXCPU, sayHello);
      for (int i = 0; i < MONITORED_ELEMENTS; i++) {
          pid_t pid = fork();

          if (pid == 0) {
				 struct rlimit cpuLimit;
             struct rlimit memoryLimit;

				 cpuLimit.rlim_cur = m[i].max_cpu_t;
             cpuLimit.rlim_max = m[i].max_cpu_t;

				 memoryLimit.rlim_cur = m[i].max_memory;
             memoryLimit.rlim_max = m[i].max_memory;

            int rct = setrlimit(RLIMIT_CPU, &cpuLimit);
            int rmt = setrlimit(RLIMIT_AS, &memoryLimit);

				if(rct || rmt) {
					printf("== Error with setting limits ==");
					return;
				}


            if (mode == load) start_monitoring_l(&m[i]);
            else if (mode == copy) start_monitoring_cp(&m[i]);
            else printf("Unknown mode value");
          }
      }


      int res;
      for (int i = 0; i < MONITORED_ELEMENTS; i++) {

          pid_t curr = wait(&res);

			 if(WEXITSTATUS(res) == 255) {
					printf("== Process %d tried to allocate more memory than allowed. ==\n\n", curr);
			 }
			 else {
          		printf("Process %d created %d file copies.\n", curr, WEXITSTATUS(res));

			 		struct rusage ch_us;
          		getrusage(RUSAGE_CHILDREN, &ch_us);

          		printf("User time: %ld.%ld\n", ch_us.ru_utime.tv_sec, ch_us.ru_utime.tv_usec);
          		printf("System time:  %ld.%ld\n\n", ch_us.ru_stime.tv_sec, ch_us.ru_stime.tv_usec);
          }
    }
}








