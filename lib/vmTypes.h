#ifndef VM_TYPES_H_
#define VM_TYPES_H_

#include "dTypes.h"

/*
    Custom type definition for virtual memory table
    Could be TLB or Page table
*/
typedef struct
{
    int *pageNumArr;  // page number array
    int *frameNumArr; // frame number array for this
    int *entryAgeArr; // Age of each index
    int length;
    int pageFaultCount;
    int tlbHitCount;
    int tlbMissCount;
} vmTable_t;

/*
    Custom type definition for page directory
*/
typedef struct
{
    vmTable_t **pageTableArr; // array of pointers to second level page tables
    int length;               // number of second level page tables
} pageDirectory_t;

/*
    Custom type definition for a process
*/
typedef struct
{
    int process_id;
    int num_addresses;
    int *addresses;
} Process;

/*
    Custom type definition for a table entry
*/
typedef struct
{
    int frame_number;
    int present;
    int caching;
    int dirty;
} vmEntry_t;

// function creates a TLB or Page table reference
vmTable_t *createVmTable(int length);

// function formats & prints content of VM table
void displayTable(vmTable_t **table);

// function frees allocated space for VM table
void freeVmTable(vmTable_t **table);

// function to allocate space in DRAM (Physical Memory)
int **createDRAM(int numFrames, int blockSize);

// function to free allocated space in DRAM
void freeDRAM(int ***dblPtrArr, int frameCount);

// function to get page number from virtual address
int getPageNumber(int mask, int value, int shift);

// function to get offset from virtual address
int getOffset(int mask, int value);

// function to create a process
Process *createProcess(int process_id, int num_addresses);

// function to free allocated space for a process
void freeProcess(Process *process);

// function to create a page directory
pageDirectory_t *createPageDirectory(int length);

#endif // VM_TYPES_H_
