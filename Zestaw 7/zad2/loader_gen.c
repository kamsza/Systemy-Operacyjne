#include "line.h"

int main(int argc, char **argv){
    
    int boxes = -1;

    if(argc != 3 && argc != 4) { printf("Wrong arguments - expected: <amount of loaders> <max box weight> [<number of boxes>]"); return 1; };
    
    int loaders_am =  atoi(argv[1]);
    if (loaders_am < 1) { printf("Wrong arguments - can not employ less than 1 loader - sorry\n"); return 1; };
    
    
    int max_box_weight = atoi(argv[2]);
    if (max_box_weight < 1) { printf("Negative weight - hmm, not here\n"); return 1; };
    
    if(argc == 4){
        boxes = atoi(argv[3]);
        if (boxes < 1)  { printf("Negative number of boxes - hmm, not here\n"); return 1; };
    }

    int status;
    char max_box_weight_str[100];
    char boxes_str[100];
    
    int pid[loaders_am];
    
    for(int loader = 0; loader < loaders_am; loader++){
        pid[loader] = fork();
        
        if(pid[loader] == 0){
            srand(time(NULL) + getpid());

            sprintf(max_box_weight_str,"%d",max_box_weight);
            
            if(boxes > 0){
                sprintf(boxes_str,"%d",boxes);
                if(execlp("./loader","./loader", max_box_weight_str, boxes_str, NULL) == -1) perror("ERROR exec:");
                exit(0);
            }
            else{
                if(execlp("./loader","./loader", max_box_weight_str, NULL) == -1) perror("ERROR exec:");
                exit(0);
            }
        }
    }

    for(int loader = 0; loader < loaders_am; loader++) waitpid(pid[loader],&status,0);
}
