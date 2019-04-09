#define _XOPEN_SOURCE 500
#define _ATFILE_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ftw.h>
#include	<sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>

char sign;
time_t date;





time_t charToTimeT(char* date) {
	/* Date format:
		DD-MM-YYYY */
	
	struct tm tmp_tm ={0,0,12,0,0,0,0,0,0};
	strptime(date,"%d-%m-%Y", &tmp_tm);
	
	time_t time = mktime(&tmp_tm);

	return time;
}





int nftwfunc(const char *filepath, const struct stat *statptr, int fileflags, struct FTW *pfwt) {
	const static double MAX_DIFF = 43200; //[s]
	double diff_sec = difftime(statptr->st_mtime, date);
  switch(sign){
  		case '>': if(!(diff_sec > MAX_DIFF)) return 0; break;
      case '=': if(!(diff_sec < MAX_DIFF && diff_sec > -MAX_DIFF)) return 0; break;
      case '<': if(!(diff_sec < -MAX_DIFF)) return 0; break;
	} 


	printf("path: %s\n", filepath);
	printf("type: ");
	switch (statptr->st_mode & S_IFMT){  
        case S_IFBLK:
            printf("block special");
            break;
        case S_IFCHR:
            printf("character special");
            break;
        case S_IFIFO:
            printf("FIFO special");
				break;
        case S_IFREG:
            printf("regular");
            break;
        case S_IFDIR:
            printf("directory");
            break;
        case S_IFLNK:
            printf("symbolic link");
            break;
    }
	printf("\n");
	printf("size: %ld\n", statptr->st_size);
	printf("access time: %s", ctime(&statptr->st_atime));
	printf("modification time: %s\n", ctime(&statptr->st_mtime));
	printf("\n");

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

		if(stat.st_mode & S_IFDIR)  search_stat(file_path);

		const static double MAX_DIFF = 43200; //[s]
		double diff_sec = difftime(stat.st_mtime, date);
		int stop = 0;
  		switch(sign){
  			case '>': if(!(diff_sec > MAX_DIFF)) stop = 1; break;
      	case '=': if(!(diff_sec < MAX_DIFF && diff_sec > -MAX_DIFF)) stop = 1;  break;
      	case '<': if(!(diff_sec < -MAX_DIFF)) stop = 1;  break;
		}    
		if(stop) continue;




		printf("path: %s\n", file_path);
		printf("type: ");
		switch (stat.st_mode & S_IFMT){  
        	case S_IFBLK:
            printf("block special");
            break;
        	case S_IFCHR:
            printf("character special");
            break;
       	 case S_IFIFO:
            printf("FIFO special");
				break;
      	  case S_IFREG:
            printf("regular");
            break;
       	 case S_IFDIR:
            printf("directory");
            break;
       	 case S_IFLNK:
            printf("symbolic link");
            break;
    }
		printf("\n");
		printf("size: %ld\n", stat.st_size);
		printf("access time: %s", ctime(&stat.st_atime));
		printf("modification time: %s\n", ctime(&stat.st_mtime));
		printf("\n");

		
	}
	
}




int main(int argc, char* argv[]) {
	if(argc != 5) {
		printf("Too few arguments. Expected: directory_path (<,=,>) date");
		return -1;
	}

   char* path = argv[1];
   sign = argv[2][0];
   date = charToTimeT(argv[3]);
	char *mode = argv[4];

	if(strcmp(mode, "nftw") == 0) search_nftw(path);
	if(strcmp(mode, "stat") == 0) search_stat(path);

   return 0;
}
