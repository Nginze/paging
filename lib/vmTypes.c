#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vmTypes.h"

vmTable_t *createVmTable(int length)
{
    vmTable_t *new_table = malloc(sizeof(vmTable_t));
    new_table->length = length;
    new_table->pageNumArr = malloc(sizeof(int) * length);
    new_table->frameNumArr = malloc(sizeof(int) * length);
    new_table->entryAgeArr = malloc(sizeof(int) * length);
    new_table->pageFaultCount = 0;
    new_table->tlbHitCount = 0;
    new_table->tlbMissCount = 0;

    // If there is not enough memory on the heap to make a call to malloc() // Notify and Exit
    if (new_table == NULL || new_table->pageNumArr == NULL || new_table->frameNumArr == NULL || new_table->entryAgeArr == NULL)
    {
        printf("Error - Could not allocate a new Virtual Memory Addressing Table!\r\n");
        exit(-1);
    }
    return new_table;
}

void freeVmTable(vmTable_t **table)
{
    if ((*table)->pageNumArr != NULL)
    {
        free((*table)->pageNumArr);
    }
    if ((*table)->frameNumArr != NULL)
    {
        free((*table)->frameNumArr);
    }
    if ((*table)->entryAgeArr != NULL)
    {
        free((*table)->entryAgeArr);
    }
    free(*table);
}

void displayTable(vmTable_t **tableToView)
{
    printf("\n********************* SEQUENCE START ****************************\n ");
    for (int i = 0; i < (*tableToView)->length; i++)
    {
        printf("Index(%d) := Page Number: %d\tFrame Number: %d\n", i, (*tableToView)->pageNumArr[i], (*tableToView)->frameNumArr[i]);
    }
    printf("\n********************* SEQUENCE END ***************************\n ");
}

int **createDRAM(int numFrames, int blockSize)
{
    int **temp;
    temp = malloc(numFrames * sizeof(int *));
    for (int i = 0; i < numFrames; i++)
    {
        temp[i] = (int *)malloc(sizeof(int) * blockSize);
        for (int j = 0; j < blockSize; j++)
        {
            temp[i][j] = 0;
        }
    }
    // If there is not enough memory to make call to malloc() // Notify and exit
    if (temp == NULL)
    {
        printf("Error - Could not allocate a new Physical Memory Matrix using dramAllocate() function!\r\n");
        exit(-1);
    }
    return temp;
}

void freeDRAM(int ***dblPtrArr, int frameCount)
{
    for (int i = 0; i < frameCount; i++)
    {
        if ((*dblPtrArr)[i] != NULL)
        {
            free((*dblPtrArr)[i]);
        }
    }
    free(*dblPtrArr);
}

int getPageNumber(int mask, int value, int shift)
{
    return ((value & mask) >> shift);
}

int getOffset(int mask, int value)
{
    return value & mask;
}


Process_t *createProcess()
{
    Process_t *newProcess = (Process_t *)malloc(sizeof(Process_t));
    newProcess->addressQueue = NULL;
    return newProcess;
}

void addAddressToProcess(Process_t *process, int address)
{
    insertAtEnd(&(process->addressQueue), address);
}
