#ifndef TYPES_H_
#define TYPES_H_

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

// This function creates a TLB or Page table reference
vmTable_t *createVmTable(int length);

// This function formats & prints content of VM table
void displayTable(vmTable_t *table);

// This function frees allocated space for VM table
void freeVmTable(vmTable_t *table);

// Function to allocate space in DRAM (Physical Memory)
void createDRAM(int numFrames, int blockSize);

#endif // TYPES_H_
