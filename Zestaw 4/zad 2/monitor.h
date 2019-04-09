#ifndef LAB3_MONITOR_H
#define LAB3_MONITOR_H

#include <time.h>

typedef struct monitor {
    char* file_name;
    char* file_path;
    char* content;
    int monitoring_freq_time;
    time_t modification_time;
    pid_t pid;
	 int state;
} monitor;


monitor* load_list(char* list);
void start_monitoring(monitor *monitor);

#endif //LAB3_MONITOR_H
