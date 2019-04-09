#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

Array* createArray(int size) {
    Array* newArray = (Array*) calloc(1,sizeof(Array));
    newArray->array = (char**) calloc((size_t)size,sizeof(char*));
    newArray->arraySize = size;
    newArray->currIndex = 0;
    newArray->tmpFile = NULL;

    return newArray;
}

void setDir(Array* array, char* dirPath) {
    array->searchDirPath = dirPath;
}

void setSearchFile(Array* array, char* fileName) {
    array->searchFile = fileName;
}

void setTmpFile(Array* array, char* fileName) {
    array->tmpFile = fileName;
}

void search(Array * array) {
    int length = strlen("find ")+strlen(array->searchDirPath)+strlen(" -name ")+
                 + 2*strlen("\"") +strlen(array->searchFile)+strlen(" > ")+strlen(array->tmpFile);

    char* instruction = (char*) calloc(length,sizeof(char));
    strcpy(instruction,"find ");
    strcat(instruction,array->searchDirPath);
    strcat(instruction," -name ");
    strcat(instruction,"\"");
    strcat(instruction,array->searchFile);
    strcat(instruction,"\"");
    strcat(instruction," > ");
    strcat(instruction,array->tmpFile);

    int res = system(instruction);
	if(res == -1) 
		printf("\n============\nSystem(instruction) failed\n============\n");
}


void setNextIndex(Array* array) {
    array->currIndex++;

    if(array->array[array->currIndex] != NULL || array->currIndex >= array->arraySize) {
        for(int i = 0; i < array->arraySize; i++) {
            if(array->array[i] == NULL) {
                array->currIndex = i;
                return;
            }
        }
        printf("\n\n Tablica pełna \n\n");
    }
}


int tmpFileToArray(Array* array) {
    FILE *file = fopen(array->tmpFile,"r");
    fseek(file,0L,SEEK_END);
    long length  = ftell(file);
    fseek(file,0L,SEEK_SET);
    char* c = (char*) calloc(length,sizeof(char));
    size_t s = fread(c,sizeof(char),length,file);
	if(sizeof(char)*length < s ) printf("\n============\nfread failed\n============\n");
    int index = array->currIndex;
    array->array[index] = c;
    setNextIndex(array);
    return index;
}

void removeBlock(Array * array, int index) {
    if(index >= array->arraySize || index < 0) {
        printf("Index out of bounds");
        return;
    }

    if(array->array[index] == NULL) return;
    free(array->array[index]);
    array->array[index] = NULL;
}

void removeArray(Array* array) {
    if(array == NULL) return;

    for(int i = 0; i < array->arraySize; i++)
        removeBlock(array, i);

    free(array->array);
    array->array = NULL;
    array->searchFile = NULL;
    strcpy(array->tmpFile, "");
    array->tmpFile = NULL;
    array->arraySize = -1;
    array->currIndex = -1;
}


void show(Array* a) {
    printf("\n\n==========================================\n");
    printf("\nShowing myself:\n arraysize:  ");
    printf("%ld",sizeof(a->array));
    printf("\nSearch directory path:  ");
    printf("%s", a->searchDirPath);
    printf("\nSearch file path:  ");
    printf("%s", a->searchFile);
    printf("\nTmp file:  ");
    printf("%s", a->tmpFile);
    printf("\nArray size:  ");
    printf("%d",a->arraySize);
    printf("\nCurrent index:  ");
    printf("%d",a->currIndex);
    printf("\n==========================================\n\n");
}



