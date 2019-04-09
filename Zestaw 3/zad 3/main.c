#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"




int main(int argc, char **argv) {
    if(argc != 6) {
        printf("Bad arguments number. Expecting: FILE TIME MODE MAX_CPU_TIME MAX_MEMORY");
        return 1;
    }

    char* list_of_files = argv[1];
    char* monitoring_freq = argv[2];

    enum mode  m;
    if(strcmp(argv[3],"load") == 0) m = load;
    else if(strcmp(argv[3],"copy") == 0) m = copy;
    else {printf("Wrong mode"); return 1;}

	int max_cpu_t = atoi(argv[4]);
	int max_memory = atoi(argv[5]);	

    monitor *monitor_tab = load_list(list_of_files, monitoring_freq, max_cpu_t, max_memory);

    start_monitoring(monitor_tab, m);


    return 0;
}
