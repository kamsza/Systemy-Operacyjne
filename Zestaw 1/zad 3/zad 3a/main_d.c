#include <time.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>

FILE* f = NULL;

typedef struct Array {
    char **array;
    char *searchDirPath;
    char *searchFile;
    char *tmpFile;
    int arraySize;
    int currIndex;
} Array;

void measureTime(char*message, const struct timespec* start, const struct timespec* end, const struct tms* startTms, const struct tms* endTms){
    double sysT = (double) (endTms->tms_stime - startTms->tms_stime) / sysconf(_SC_CLK_TCK);
    double userT = (double) (endTms->tms_utime - startTms->tms_utime) / sysconf(_SC_CLK_TCK);
    double realT = (double) (end->tv_sec - start->tv_sec);
    realT += (double) (end->tv_nsec - start->tv_nsec) / 1e9;

    // times saved in milliseconds
    realT*=1000.0;
    sysT*=1000.0;
    userT*=1000.0;

    char str[512];

    sprintf(str,strcat(message,"\n\tSystem time: %.10lf ms \n\tUser time: %.10lf ms \n\tReal time: %.10lf ms \n"),sysT,userT,realT);
    printf("%s",str);

    fprintf(f, "%s\n",str );
    fflush(f);
}



int main(int argc, char* argv[]) {
	struct tms startTmsProg, stopTmsProg;
    struct timespec startProg, stopProg;

	clock_gettime(CLOCK_REALTIME, &startProg);
    times(&startTmsProg);

	void* lib = NULL;
	lib = dlopen("./lib.so", RTLD_LAZY);

	if (!lib) {
      printf("\n================\nError during library linking\n================\n");
      exit(1);
    }


	Array* (*createArray) (int size);
	void (*setDir) (Array* array, char* dirName);
	void (*setSearchFile) (Array* array, char* fileName);
	void (*setTmpFile) (Array* array, char* fileName);
	void (*search) (Array* array);
	int (*tmpFileToArray) (Array* array);
	void (*removeBlock) (Array * array, int Index);
	void (*removeArray) (Array* array);

	createArray = dlsym(lib, "createArray"); 
	setDir = dlsym(lib, "setDir");
	setSearchFile = dlsym(lib, "setSearchFile"); 
	setTmpFile = dlsym(lib, "setTmpFile");
	search = dlsym(lib, "search");
	tmpFileToArray = dlsym(lib, "tmpFileToArray");
	removeBlock = dlsym(lib, "removeBlock");
	removeArray = dlsym(lib, "removeArray"); 

	if (!createArray || !setDir || !setSearchFile || !setTmpFile || !search || !tmpFileToArray || !removeBlock || !removeArray) {
      printf("\n================\nError during functions linking\n================\n\n");
      exit(1);
    }

    f = fopen("raport dynamic", "a");

    Array *blockArr = NULL;
    struct tms startTms, stopTms;
    struct timespec start, stop;

    char message[256];

    int i = 1;


    while(i < argc) {
        clock_gettime(CLOCK_REALTIME, &start);
        times(&startTms);

        if(strcmp(argv[i], "create_table") == 0) {
            if(i + 1 > argc) {printf("Table size undefined") ; break;}
            if(blockArr != NULL) {printf("This test program can handle only one array") ; break;}

            i++;
            char* sizeStr = argv[i];
            int size = atoi(sizeStr);
            blockArr = createArray(size);

            clock_gettime(CLOCK_REALTIME,&stop);
            times(&stopTms);

            sprintf(message, "Array with %d blocks created.", size);
            measureTime(message, &start,&stop,&startTms,&stopTms);

        }
        else if(strcmp(argv[i], "search_directory") == 0) {
            if(i + 3 > argc) {printf("Search directory arguments undefined") ; break;}

            i++;
            setDir(blockArr, argv[i]);
            i++;
            setSearchFile(blockArr, argv[i]);
            i++;
            setTmpFile(blockArr, argv[i]);

            clock_gettime(CLOCK_REALTIME,&stop);
            times(&stopTms);

            search(blockArr);
            int index = tmpFileToArray(blockArr);

            sprintf(message, "Search of file %s in %s successful. Result index %d.", blockArr->searchFile, blockArr->searchDirPath, index);
            measureTime(message,&start,&stop,&startTms,&stopTms);
        }
        else if(strcmp(argv[i], "remove_block") == 0) {
            if(i + 1 > argc) {printf("Removed block index undefined") ; break;}

            i++;
            char* indexStr = argv[i];
            int index = atoi(indexStr);
            removeBlock(blockArr, index);

            clock_gettime(CLOCK_REALTIME,&stop);
            times(&stopTms);

            sprintf(message, "Block with index %d removed.", index);
            measureTime(message,&start,&stop,&startTms,&stopTms);
        }
        else {
            printf("Undefined method: %s at %d position", argv[i], i) ;
            break;
        }
        i++;
    }



    removeArray(blockArr);
    fclose(f);

	dlclose(lib);

	clock_gettime(CLOCK_REALTIME,&stopProg);
    times(&stopTmsProg);
	sprintf(message, "Długość działania programu");
	measureTime(message,&startProg,&stopProg,&startTmsProg,&stopTmsProg);
}
