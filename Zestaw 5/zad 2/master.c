#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>




int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Wrong arguments. Program expects: <FIFO file name>\n");
        return 1;
    }

	 char* file_name = argv[1];
	 int res = mkfifo(file_name, S_IRUSR | S_IWUSR);

    if(res < 0) {
        printf("mkfifo error  \n");
        return 1;
    }

    char *buff = calloc(64, sizeof(char));
    int file = open(file_name, O_RDONLY);

    if(file < 0){
        printf("Couldn't open file: %s \n", argv[1]);
        return 1;
    }

    while (read(file, buff, 64)) {
        printf("%s", buff);
    }

    free(buff);
    close(file);
    return 0;
}
