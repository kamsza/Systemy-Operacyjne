#include <stdio.h>
#include <string.h>
#include "monitor.h"




int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Bad arguments number. Expecting: <FILE WITH LIST>");
        return 1;
    }


    monitor *monitor_tab = load_list(argv[1]);

    start_monitoring(monitor_tab);


    return 0;
}
