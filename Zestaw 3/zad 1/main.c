#define _XOPEN_SOURCE 500
#define _ATFILE_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ftw.h>
#include	<sys/stat.h>
#include <sys/types.h>
#include <unistd.h> 
#include <dirent.h>
#include <fcntl.h>
#include<sys/wait.h> 

char sign;
time_t date;



int nftwfunc(const char *filepath, const struct stat *statptr, int fileflags, struct FTW *pfwt) {

	if (S_ISDIR(statptr->st_mode)){   
			pid_t pid = fork();
			if(pid == 0) {
				printf("path: %s \n", filepath);
				printf("pid:  %d \n", (int)getpid());
				execlp("/bin/ls", "ls", "-l", filepath, NULL);
				exit(0);
			}
			else {
				wait(0);
			}
	}
	return 0;
}







void search_nftw(const char * dir_path){
	int fd_limit = 128; 	
	int flags = FTW_PHYS;

   int result = nftw(dir_path, nftwfunc, fd_limit, flags);
	
	if(result != 0) printf("\n\n========\nnftw ended with error \n========\n\n");
}




void search_stat(const char * dir_path){
	DIR *directory = NULL;
   directory = opendir(dir_path);
	
	if(directory == NULL) {
		printf("Can't open given directory in search() function");
		return;
	}

   struct stat stat;
   struct dirent *file = NULL;

	while((file = readdir(directory))) {
		if(strcmp(file->d_name,".") == 0 || strcmp(file->d_name,"..") == 0) continue;

   	char file_path[1024];
		sprintf(file_path,"%s/%s",dir_path,file->d_name);

		lstat(file_path, &stat);

		if (S_ISDIR(stat.st_mode)){   
			pid_t pid = fork();
			if(pid == 0) {
				printf("path: %s \n", file_path);
				printf("pid:  %d \n", (int)getpid());
				execlp("/bin/ls", "ls", "-l", file_path, NULL);
				exit(0);
			}
			else {
				wait(0);
				search_stat(file_path);
			}
		}
	}
	
}






int main(int argc, char* argv[]) {
	if(argc != 3) {
printf("%d\n", argc);
		printf("Too few arguments. Expected path and mode\n");
		return -1;
	}

   char* path = argv[1];
	char* mode = argv[2];

	if(strcmp(mode, "nftw") == 0) search_nftw(path);
	if(strcmp(mode, "stat") == 0) search_stat(path);

   return 0;
}
