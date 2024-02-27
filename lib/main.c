#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "types.h"

#define VIRTUAL_MEMORY_SIZE (1 << 30)                            // 1GB
#define PHYSICAL_MEMORY_SIZE (1 << 28)                           // 256MB
#define PAGE_SIZE (1 << 12)                                      // 4KB
#define MAX_PAGE_TABLE_ENTRIES (VIRTUAL_MEMORY_SIZE / PAGE_SIZE) // 2^18 entries
#define MAX_FRAME_ENTRIES (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)     // 2^16 entries
#define MAX_TLB_ENTRIES 16                                       // 16 entries

/*
    File:           main.c
    Date:           29 Feb 2024
    Course:         Operating Systems
    Assignment:     Midsemester Project
*/

vmTable_t *pageTable;
vmTable_t *tlbTable;

// Define (Hoist) utility prototypes
void translateAddress();
void tlbInsertFIFO();
void tlbInsertLRU();

int main(int argc, char const *argv[])
{
    //@TODO: Implement main program

    // *Initialize page table, TLB and physical memory (DRAM)
    // *Find way to simulate processes accessing memory (allocating and deallocating memory)
    // *Perform vpn translation to ppn
    // *Use various page replacement algorithms (LRU and FIFO)
    // *Use various TLB replacement algorithms (LRU and FIFO)
    // *Report statistics

    // *Once base works, Implement multilevel page table
    // *Develop CLI program to interact with the system

    tlbTable = createVmTable(MAX_TLB_ENTRIES);
    pageTable = createVmTable(MAX_PAGE_TABLE_ENTRIES); // @TODO: Change to PD (Page Directory)

    // @TODO: Simulate processes here
    return 0;
}

void translateAddress()
{
}

void tlbInsertFIFO()
{
}

void tlbInsertLRU()
{
}
