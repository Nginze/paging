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

Process *createProcess(int process_id, int num_addresses)
{
    Process *process = (Process *)malloc(sizeof(Process));
    if (process == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for process\n");
        exit(EXIT_FAILURE);
    }
    process->process_id = process_id;
    process->num_addresses = num_addresses;
    process->addresses = (int *)malloc(num_addresses * sizeof(int));
    if (process->addresses == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for process addresses\n");
        exit(EXIT_FAILURE);
    }
    return process;
}

void freeProcess(Process *process)
{
    free(process->addresses);
    free(process);
}

pageDirectory_t *createPageDirectory(int length)
{
    pageDirectory_t *new_directory = malloc(sizeof(pageDirectory_t));
    new_directory->length = length;
    new_directory->pageTableArr = malloc(sizeof(vmTable_t *) * length);

    // If there is not enough memory on the heap to make a call to malloc() // Notify and Exit
    if (new_directory == NULL || new_directory->pageTableArr == NULL)
    {
        printf("Error - Could not allocate a new Page Directory!\r\n");
        exit(-1);
    }

    // Initialize all pointers to NULL
    for (int i = 0; i < length; i++)
    {
        new_directory->pageTableArr[i] = NULL;
    }

    return new_directory;
}
