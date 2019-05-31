#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <libgen.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <math.h>


enum div_method {block, interleaved};

int threads_num;

int image_width;
int image_height;
int max_pix_val;

int filter_size;

int **img;
double **filter;
int **result;



int** load_image(char * file_path);
double** load_filter(char * file_path);
int** init_result();

double* calc_interleaved(void* args);
double* calc_blocked(void* args);

void write_result(char* file_path);
void clear();






int main(int argc, char *argv[]) {
    if(argc != 6) {
        printf("Wrong arguments - expected <number of threads> <method of dividing the picture (b/i)> <input file name> <filter file name> <output file name>\n");
        return 1;
    }

    threads_num = atoi(argv[1]);

    enum div_method method;

    if(strcmp(argv[2], "b") == 0) method = block;
    else if(strcmp(argv[2], "i") == 0) method = interleaved;
    else { printf("Method %s not found /n", argv[2]); return -1; }


    char *img_filename = argv[3];
    char *filter_filename = argv[4];
    char *output_filename = argv[5];
    
    img = load_image(img_filename);
    filter = load_filter(filter_filename);
    result = init_result();


    clock_t begin = clock();

    pthread_t thread[threads_num];

    for(int i = 0; i < threads_num; i++) {
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        if ( arg == NULL ) { perror("ERROR malloc"); exit(1); }
        switch(method) {
            case block:
                pthread_create(&thread[i], NULL, (void*)calc_blocked, arg);
                break;
            case interleaved:
                pthread_create(&thread[i], NULL, (void*)calc_interleaved, arg);
                break;
        }

    }


    for(int i = 0; i < threads_num; i++) {
			void *returned_time;
   		pthread_join(thread[i], &returned_time);
    		printf("Thread %d  tid: %ld    time: %.6lf \n", i, thread[i], *(double *)returned_time);
    }

    clock_t end = clock();
    double total_time = (double)(end - begin)/ CLOCKS_PER_SEC;

    printf("Total time: %.6lf\n", total_time);

    write_result(output_filename);

    clear();

    return 0;
}





// LOAD 

int** load_image(char * file_path) {

    FILE *fp = fopen(file_path, "r");
    if(fp == NULL) { perror("ERROR couldn't open image file"); exit(1); }

    char header[5];
    fgets ( header, sizeof header, fp );
    if(header == NULL) printf("ERROR fgets in load_image \n");

//    if(strcmp(header, "P2\n") != 0 || strcmp(header, "P2") != 0) {
//        printf("ERROR in load_image: first line of file: %s, expected value: P2", header);
//        exit(1);
//    }

    fscanf(fp, "%d", &image_width);
    fscanf(fp, "%d", &image_height);
    fscanf(fp, "%d", &max_pix_val);

    int **img = calloc(image_height, sizeof(int *));

    for(int h = 0; h < image_height; h++) {
        img[h] = calloc(image_width, sizeof(int));

        for(int w = 0; w < image_width; w++)
            fscanf(fp, "%d", &img[h][w]);
    }

    fclose(fp);
    return img;
}

double** load_filter(char * file_path) {
    FILE *fp = fopen(file_path, "r");
    if(fp == NULL) { perror("ERROR couldn't open filter file:"); exit(1); }

    fscanf(fp, "%d", &filter_size);
    double** filter = calloc(filter_size, sizeof(double*));

    for(int h = 0; h < filter_size; h++) {
        filter[h] = calloc(filter_size, sizeof(double));
        
        for(int w = 0; w < filter_size; w++)
            fscanf(fp, "%lf", &filter[h][w]);
    }

    fclose(fp);
    return filter;
}


int** init_result() {
    int **res = calloc(image_height, sizeof(int *));

    for(int h = 0; h < image_height; h++)
        res[h] = calloc(image_width, sizeof(int));

    return res;
}


// CALCULATE

int new_value(int x, int y) {
    double s = 0;
    int s_x, s_y;

    for(int i = 0; i < filter_size; i++)
    for(int j = 0; j < filter_size; j++) {
        s_x = x - ceil(filter_size / 2.0) + i;
        if(s_x < 0) s_x = 0;
        s_y = y - ceil(filter_size / 2.0) + j;
        if(s_y < 0) s_y = 0;
        s += s_y < image_height && s_x < image_width ? img[s_y][s_x] * filter[j][i] : 0;
    }

    return round(s);
}


double* calc_interleaved(void* args) {
    int k = *((int*)args);

    clock_t begin = clock();

    for(int y = 0;y < image_height; y++)
        for(int x = k; x < image_width; x += threads_num)
        result[y][x] = new_value(x, y);

    clock_t end = clock();

	double* time = malloc(sizeof(double));
	*time = (double)(end - begin)/ CLOCKS_PER_SEC;

    pthread_exit(time);
}



double* calc_blocked(void* args) {

    int k = *((int*) args);

    int from = ceil(k * image_width / ((double) threads_num));
    int to = ceil((k + 1) * image_width / ((double) threads_num)) - 1;

    clock_t begin = clock();

    for(int y = 0; y < image_height; y++)
    for(int x = from; x <= to; x++)
            result[y][x] = new_value(x, y);

    clock_t end = clock();

    double* time = malloc(sizeof(double));
	 *time = (double)(end - begin)/ CLOCKS_PER_SEC;

    pthread_exit(time);
}




// END

void write_result(char *file_path) {
    FILE *fp = fopen(file_path, "w");
    if(fp == NULL) { perror("ERROR couldn't open result file:"); exit(1); }
    
    
    fprintf(fp, "P2\n%d %d\n%d\n", image_width, image_height, 255);
    
    for(int h = 0; h < image_height - 1; h++) {
        for(int w = 0; w < image_width; w++) 
            fprintf(fp, "%d ", result[h][w]);
        
        fprintf(fp, "\n");
    }
    for(int w = 0; w < image_width; w++) 
        fprintf(fp, "%d ", result[image_height - 1][w]);
    
    fclose(fp);
}



void clear() {

    for(int h = 0; h < image_height; h++)
        free(img[h]);
    free(img);

    for(int h = 0; h < filter_size; h++)
        free(filter[h]);
    free(filter);

    for(int h = 0; h < image_height; h++)
        free(result[h]);
    free(result);

}
