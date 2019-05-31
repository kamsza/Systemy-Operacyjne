#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>

char* num_of_threads[] = {"1", "2", "4", "8"};
char* div_method[] = {"b", "i"};
char* image;
char* filter[100];
int filters_num = 0;


void read_filters(char *directory_name);
char* generate_name(char*directory, char* name, int n, int dm, int fn);


int main(int argc,char** argv){
    if(argc != 2) { printf("Wrong arguments - expected name of image that exists in images directory"); return 1; };
	
	 image = malloc(sizeof("images/") + sizeof(argv[1]));
    strcpy(image, "images/");
    strcat(image, argv[1]);

	 FILE* file;
	 if ((file = fopen(image, "r"))){ fclose(file); }
	 else { printf("Image: %s not found in images folder", argv[1]); return 1; }


    read_filters("filters");


	 for(int f = 0; f < filters_num; f++)
    for(int n = 0; n < 4; n++)
    for(int dm = 0; dm < 2; dm++)
     {
        char *output_file_name = generate_name("results", "output_image", n, dm, f);
        char *result_file_name = generate_name("results", "result_time", n, dm, f);


        pid_t pid = fork();
        if (pid == 0) {
            int fd = open(result_file_name, O_RDWR | O_CREAT, 0666);
            dup2(fd, 1);
            dup2(fd, 2);
            close(fd);

            printf("Image: %s \nFilter: %s \nThreads: %s \nDiv method: %s \n", image, filter[f], num_of_threads[n],div_method[dm]);
            fflush(stdout);

            execlp("./main", "./main", num_of_threads[n], div_method[dm], image, filter[f], output_file_name, NULL);
            perror("ERROR execlp");
            exit(1);
        }
    }

	for(int i = 0; i < 8*filters_num; i++)
		wait(NULL);

    return 0;
}


void read_filters(char *directory_name) {
    struct dirent *de;
    DIR *dr = opendir(directory_name);
    if (dr == NULL) { printf("Could not open %s directory",  directory_name); return; }

    while ((de = readdir(dr)) != NULL) {
        if( strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 ) continue;
        filter[filters_num] = malloc(sizeof("filters/") + sizeof(de->d_name));
        sprintf(filter[filters_num], "filters/%s", de->d_name);
        filters_num++;
    }

    closedir(dr);
}


char* generate_name(char*directory, char* name, int n, int dm, int fn) {

    char* output_file_name = malloc(sizeof(directory) + sizeof(name) + 10);
    char* num = malloc(16*sizeof(char));
    sprintf(num, "%s_%d_%s", num_of_threads[n], fn, div_method[dm]);

    strcpy(output_file_name, directory);
    strcat(output_file_name, "/");
	 strcat(output_file_name, name);
	 strcat(output_file_name, "_");
    strcat(output_file_name, num);
	 printf("%s\n", output_file_name);
    return output_file_name;
}
