#include "../zad1/lib.h"
#include <time.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>

FILE* f = NULL;

void measureTime(char*message, const struct timespec* start, const struct timespec* end, const struct tms* startTms, const struct tms* endTms){
    double sysT = (double) (endTms->tms_stime + endTms->tms_cstime - startTms->tms_stime - startTms->tms_cstime) / sysconf(_SC_CLK_TCK);
    double userT = (double) (endTms->tms_utime + endTms->tms_cutime - startTms->tms_utime - startTms->tms_cutime) / sysconf(_SC_CLK_TCK);
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
    Array *blockArr = NULL;
    struct tms startTms, stopTms;
    struct timespec start, stop;

	struct tms startTmsProg, stopTmsProg;
    struct timespec startProg, stopProg;

    f = fopen("raport", "a");

    char message[256];
    int i = 1;
	clock_gettime(CLOCK_REALTIME, &startProg);
    times(&startTmsProg);

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

	clock_gettime(CLOCK_REALTIME,&stopProg);
    times(&stopTmsProg);
	sprintf(message, "Długość działania programu");
	measureTime(message,&startProg,&stopProg,&startTmsProg,&stopTmsProg);

    removeArray(blockArr);
    fclose(f);

}
