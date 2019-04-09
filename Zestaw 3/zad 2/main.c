#include <stdio.h>
#include <string.h>
#include "monitor.h"




int main(int argc, char **argv) {
    if(argc != 4) {
        printf("Bad arguments number. Expecting: FILE TIME MODE");
        return 1;
    }

    char* list_of_files = argv[1];
    char* monitoring_freq = argv[2];
    enum mode  m;
    if(strcmp(argv[3],"load") == 0) m = load;
    else if(strcmp(argv[3],"copy") == 0) m = copy;
    else {printf("Wrong mode"); return 1;}

    monitor *monitor_tab = load_list(list_of_files, monitoring_freq);

    start_monitoring(monitor_tab, m);


    return 0;
}