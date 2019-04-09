#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <zconf.h>
#include <wait.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>



int main(int argc, char **argv) {
    if (argc != 5)
        printf("== Wrong arguments ==\n == Expecting 4 arguments <plik> <pmim> <pmax> <bytes> ==");

    srand(time(NULL));
    char *file = argv[1];
    int pmin   = atoi(argv[2]);
    int pmax   = atoi(argv[3]);
    int bytes  = atoi(argv[4]);
    char date[50];
    char* random_bits = malloc(bytes + 1);

	 FILE *fp = fopen(file, "a");
	 if(fp == NULL) {
		printf("Can't open the file\n");
		return 1;
	 }

	char line[512];

    while (1) {
        int rand_sec = rand() % (pmax - pmin) + pmin;
        time_t time;
        strftime(date, 50, "%d-%m-%Y_%H-%M-%S", localtime(&time));

        for(int i=0 ; i<bytes; i++) random_bits[i] = (rand() % 26) + 97;
        random_bits[bytes] = '\n';

        sprintf(line, "pid: %d   rand time: %d   actual date: %s    bits: %s\n",getpid(), rand_sec, date, random_bits); 

       fseek(fp, 0, SEEK_END);
    	 fputs(line, fp);
		 fputs("\n", fp);

       sleep(rand_sec);
    } 
 	fclose(fp);
	free(random_bits);
}

