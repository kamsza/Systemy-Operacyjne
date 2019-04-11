#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>


int getArgAsInt(char ** argv, int id){
    return (int) strtol(argv[id], NULL, 10);
}


int main(int argc, char **argv) {
    srand(time(0));
    if (argc != 3) {
		  printf("Wrong arguments. Program expects: <FIFO file name> <number of lines to send> \n");
        return 1;
    }

    int file = open(argv[1], O_WRONLY);
    if(file < 0){
        printf("Couldn't open file: %s \n", argv[1]);
        return 1;
    }

	 int to_send = atoi(argv[2]);
	 if(to_send < 0) {
		  printf("Second argument can't be a negative number.  \n");
        return 1;
	 }

	 int PID = getpid();
	 printf("Slave pid: %d\n", PID);

	 char date[64];
	 char buff[64];

     while(to_send--) {
        FILE *dateF = popen("date", "r");
			if(!dateF) {
        		printf("popen returned null pointer");
        		return 1;
    		}
		   fread(date, sizeof(char), 64, dateF);
			fclose(dateF);
         sprintf(buff, "%d\t%s", PID, date);
       	int res = write(file, buff, 64);
			if(res < 0) {
				printf("write error");
				return 1;
			}
        sleep(rand()%4+2);
    }
    close(file);
    return 0;
}




