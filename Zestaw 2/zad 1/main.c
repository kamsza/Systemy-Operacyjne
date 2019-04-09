#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include<fcntl.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/stat.h>
#include <sys/times.h>





void generate(char * file_name, int record_number, int record_size) {
    FILE *file = fopen(file_name, "w");
    if (file == NULL) {
        printf("Nie można otworzyć pliku");
        exit(1);
    }

    srand(time(NULL));
    char *record = (char *) calloc(record_size, sizeof(char));

    for (int i = 0; i < record_number; i++) {
        for (int i = 0; i < record_size - 1; i++)
            record[i] = rand() % 25 + 65;
        record[record_size - 1] = '\n';
        fprintf(file, "%s", record);
    }
    
	 free(record);
    fclose(file);
}





void sort_sys(char * file_name, int record_number, int record_size) {
	int file = open(file_name, O_RDWR);

	char * minimum  = (char *)calloc(record_size, sizeof(char));
	char * current = (char *)calloc(record_size, sizeof(char));
	int min_index;

	for(int i = 0; i < record_number - 1; i++) {
		lseek(file, i * record_size, SEEK_SET);
		read(file, minimum, sizeof(char) * record_size);
		min_index = i;

		for(int j = i + 1; j < record_number; j++) {
			lseek(file, j * record_size, SEEK_SET);
			read(file, current, sizeof(char) * record_size);

			if(current[0] < minimum[0]) {
				strcpy(minimum, current);
				min_index = j;
			}
		}

		if(min_index != i) {
			lseek(file, i * record_size,SEEK_SET);
			read(file, current, sizeof(char) * record_size);
			lseek(file, i * record_size,SEEK_SET);
			write(file, minimum, sizeof(char) * record_size);
			lseek(file, min_index * record_size,SEEK_SET);
			write(file, current, sizeof(char) * record_size);
		}
	}

	free(minimum);
	free(current);
	close(file);
}





void sort_lib(char * file_name, int record_number, int record_size) {
	FILE* file = fopen(file_name, "r+");
	
	char* minimum  = (char *)calloc(record_size, sizeof(char));
	char* current = (char *)calloc(record_size, sizeof(char));
	int min_index;


	for(int i = 0; i < record_number - 1; i++) {
		fseek(file, i * record_size,SEEK_SET);
		fread(minimum, sizeof(char), record_size, file);
		min_index = i;

		for(int j = i + 1; j < record_number; j++) {
			fseek(file, j * record_size, SEEK_SET);
			fread(current, sizeof(char), record_size, file);

			if(current[0] < minimum[0]) {
				strcpy(minimum, current);
				min_index = j;
			}
		}

		if(min_index != i) {
			fseek(file, i * record_size,SEEK_SET);
			fread(current, sizeof(char), record_size, file);
			fseek(file, i * record_size,SEEK_SET);
			fwrite(minimum, sizeof(char), record_size, file);
			fseek(file, min_index * record_size,SEEK_SET);
			fwrite(current, sizeof(char), record_size, file);
		}
	}

	free(minimum);
	free(current);
	fclose(file);
}





void sort(char * file_name, int record_number, int record_size, char* mode){
	if(strcmp(mode,"sys")==0) sort_sys(file_name, record_number, record_size);
   else if(strcmp(mode,"lib")==0) sort_lib(file_name, record_number, record_size);
	else {printf("Błąd przy wyborze wariantu działania funkcji"); exit(1);}
}





void copy_sys(char * from_file_name, char * to_file_name, int record_number, int record_size) {
	int file_from = open(from_file_name, O_RDONLY);
	int file_to = open(to_file_name, O_WRONLY | O_CREAT);

	char* buffer = calloc(record_size, sizeof(char));

	for(int i = 0; i < record_number; i++) {
		read(file_from, buffer, sizeof(char) * record_size);
		write(file_to, buffer, sizeof(char) * record_size);
	} 

	close(file_from);
	close(file_to);
	free(buffer);
}





void copy_lib(char * from_file_name, char * to_file_name, int record_number, int record_size) {
	 FILE* file_from = fopen(from_file_name, "r");
    FILE* file_to = fopen(to_file_name, "a");
	
	if(file_to == NULL) printf("\n\n ============ ;( ============ \n\n");

	char* buffer = calloc(record_size, sizeof(char));

	for(int i = 0; i < record_number; i++) {
		fread(buffer, sizeof(char), record_size, file_from);
		fwrite(buffer, sizeof(char), record_size, file_to);
	}

   fclose(file_from);
   fclose(file_to);
	free(buffer);
}





void copy(char * from_file_name, char * to_file_name, int record_number, int record_size, char* mode){
	if(strcmp(mode,"sys")==0)  copy_sys(from_file_name, to_file_name, record_number, record_size);
   else if(strcmp(mode,"lib")==0) copy_lib(from_file_name, to_file_name, record_number, record_size);
	else {printf("Błąd przy wyborze wariantu działania funkcji"); exit(1);}
}





int main(int argc, char **argv) {
    if(argc < 1) {printf("Nie podano argumentów"); return 1;}

	
    static struct tms start;
    static struct tms end;
	 
	char* func = argv[1];
	int record_number;
	int record_size;
	char* file_name;
	char* to_file;
	char* mode;

    times(&start);

    if(strcmp(argv[1],"generate")==0) {
        if(argc < 4) {printf("Nie podano wszystkich argumentów"); return 1;}

			file_name = argv[2];
			record_number = atoi(argv[3]);
			record_size = atoi(argv[4]);
			mode = "";

        generate(file_name, record_number, record_size);
    }
    else if(strcmp(argv[1],"sort")==0) {
        if(argc < 5) {printf("Nie podano wszystkich argumentów"); return 1;}

			file_name = argv[2];
			record_number = atoi(argv[3]);
			record_size = atoi(argv[4]);
			mode = argv[5];

        sort(file_name, record_number, record_size, mode);
    }
    else if(strcmp(argv[1],"copy")==0) {
        if(argc < 6) {printf("Nie podano wszystkich argumentów"); return 1;}

			file_name = argv[2];
			to_file = argv[3];
			record_number = atoi(argv[4]);
			record_size = atoi(argv[5]);
			mode = argv[6];

        copy(file_name, to_file, record_number, record_size, mode);
    }

	times(&end);


	 printf("%s %s  %d records  %d bytes\n", mode, func, record_number, record_size);
	 printf("user time: %.16LF [s]    system time: %.16LF [s]\n\n\n", 
				(long double)(end.tms_utime - start.tms_utime)/sysconf(_SC_CLK_TCK),
				(long double)(end.tms_stime - start.tms_stime)/sysconf(_SC_CLK_TCK));

	 FILE* wyniki = fopen("wyniki", "a");
	 fprintf(wyniki, "%s %s  %d records  %d bytes\n", mode, func, record_number, record_size);
	 fprintf(wyniki, "user time: %.16LF [s]    system time: %.16LF [s]\n\n\n", 
				(long double)(end.tms_utime - start.tms_utime)/sysconf(_SC_CLK_TCK),
				(long double)(end.tms_stime - start.tms_stime)/sysconf(_SC_CLK_TCK));

	 fclose(wyniki);
	 

    return 0;
}





