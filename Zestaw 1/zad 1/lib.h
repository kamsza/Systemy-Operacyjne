#ifndef SysOp_lab1_zad1
#define SysOp_lab1_zad1

#include <stdio.h>

typedef struct Array {
    char **array;
    char *searchDirPath;
    char *searchFile;
    char *tmpFile;
    int arraySize;
    int currIndex;
} Array;


Array* createArray(int size);
void setDir(Array* array, char* dirName);
void setSearchFile(Array* array, char* fileName);
void setTmpFile(Array* array, char* fileName);
void search(Array* array);
int tmpFileToArray(Array* array);
void removeBlock(Array * array, int Index);
void removeArray(Array* array);

#endif


