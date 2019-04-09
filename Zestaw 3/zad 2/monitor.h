#ifndef LAB3_MONITOR_H
#define LAB3_MONITOR_H

#include <time.h>

typedef struct monitor {
    char* file_name;
    char* file_path;
    char* content;
    int monitoring_time;
    int monitoring_freq_time;
    time_t modification_time;

} monitor;

enum mode {load, copy};


monitor* load_list(char* list, char* monitoring_time);
void start_monitoring(monitor *monitor, enum mode m);
void show_monitor(monitor monitor);



#endif //LAB3_MONITOR_H
