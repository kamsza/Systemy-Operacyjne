#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>


int main(int argc,char** argv){
    if(argc != 3) { printf("Wrong arguments - expected <filter name> <filter size>"); return 1; };

    srand(time(NULL));

    char *file_name = malloc(sizeof("filters/") + sizeof(argv[1]));
    strcpy(file_name, "filters/");
    strcat(file_name, argv[1]);

    FILE * fp = fopen(file_name,"w");
    int size = atoi(argv[2]);

    fprintf(fp,"%d \n",size);

    float *tab = calloc(size*size, sizeof(float));
    double sum = 0.0;

    for(int i = 0; i < size *size; i++) {
        tab[i] = fmodf(rand(), 100.0);
        sum += (double)tab[i];
    }

    for(int i = 0; i < size *size; i++) {
        tab[i] = tab[i] / sum;
    }

    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++)
            fprintf(fp, "%f ", tab[i * j]);
        fprintf(fp, "\n");
    }


    return 0;
}
