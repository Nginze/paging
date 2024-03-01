#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "vmTypes.h"
#include "dTypes.h"

/*
    File:           main.c
    Date:           1 March 2024
    Course:         Operating Systems
    Assignment:     Midsemester Project
*/

#define VIRTUAL_ADDRESS_SPACE (1 << 16)                         // 2^16 KB
#define PHYSICAL_ADDRESS_SPACE (1 << 16)                        // 2^16 KB
#define PAGE_SIZE (1 << 8)                                      // 256B Size of each page
#define FRAME_SIZE (1 << 8)                                     // 256B Size of each frame
#define PAGE_TABLE_SIZE (VIRTUAL_ADDRESS_SPACE / PAGE_SIZE)     // size of the page table
#define TOTAL_FRAME_COUNT (PHYSICAL_ADDRESS_SPACE / FRAME_SIZE) // total number of frames in physical memory
#define PAGE_MASK 0xFF00                                        // mask to reveal page number
#define OFFSET_MASK 0xFF                                        // mask to reveal page offset
#define SHIFT 8                                                 // bitmask shift amount for page number
#define TLB_SIZE 16                                             // size of the TLB (cache)
#define MAX_ADDR_LEN 10                                         // buffer len for random addresses (32 bit)
#define PAGE_READ_SIZE 256                                      // buffer for reading page binaries from disk file

// Simulation vars and structures
vmTable_t *tlbTable;  // translation lookup buffer for caching
vmTable_t *pageTable; // page directory (1st level page table)
int **dram;           // dynamic random access memory (physical memory)

// read buffers for address and disk access
char addressReadBuffer[MAX_ADDR_LEN];
signed char fileReadBuffer[PAGE_READ_SIZE];

// parameters for a specific virtual address instance
int virtual_addr;
int page_number;
int offset_number;
signed char translatedValue;

// pointer to track next available slot in TLB, Page Tables and DRAM
int nextTLBentry = 0;
int nextPage = 0;
int nextFrame = 0;

// pointers to file descriptors for reading addresses and disk file
FILE *address_file;
FILE *backing_store;

// CLI & Menu vars and structures
char algo_choice;    // LRU or FIFO policy
char display_choice; // option to log translation at every instance

clock_t start, end;        // start and end time
double cpu_time_used;      // execution time
int functionCallCount = 0; // number of function calls made

// hoist function prototypes
void translateAddress();
void readFromStore(int pageNumber);
void tlbFIFOinsert(int pageNumber, int frameNumber);
void tlbLRUinsert(int pageNumber, int frameNumber);
int getOldestEntry(int tlbSize);
double getAvgTimeInBackingStore();

int main(int argc, char *argv[])
{
    // initialize cache, page tables and physical memory
    tlbTable = createVmTable(TLB_SIZE);
    pageTable = createVmTable(PAGE_TABLE_SIZE);
    dram = createDRAM(TOTAL_FRAME_COUNT, FRAME_SIZE);

    // prep useful statistics
    int translationCount = 0;
    char *algoName;

    // ensure input address file with random addresses is passed
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./vm_sim [input file]\n");
        return -1;
    }

    // ensure disk file is open and ready for reading (binary)
    backing_store = fopen("./data/disk_file.bin", "rb");

    if (backing_store == NULL)
    {
        fprintf(stderr, "Error opening BACKING_STORE.bin %s\n", "BACKING_STORE.bin");
        return -1;
    }

    // ensure address file is open and ready for reading
    address_file = fopen(argv[1], "r");

    if (address_file == NULL)
    {
        fprintf(stderr, "Error opening %s. Expecting [InputFile].txt or equivalent.\n", argv[1]);
        return -1;
    }

    printf("\nWelcome to the OS Paging Simulator");
    printf("\nNumber of logical pages: %d\nPage size: %d bytes\nPage Table Size: %d\nTLB Size: %d entries\nNumber of Physical Frames: %d\nPhysical Memory Size: %d bytes", PAGE_TABLE_SIZE, PAGE_READ_SIZE, PAGE_TABLE_SIZE, TLB_SIZE, FRAME_SIZE, PAGE_READ_SIZE * FRAME_SIZE);

    do
    {
        printf("\nDisplay All Physical Addresses? [y/n]: ");
        scanf("\n%c", &display_choice);
    } while (display_choice != 'n' && display_choice != 'y');

    do
    {
        printf("Choose TLB Replacement Strategy [1: FIFO, 2: LRU]: ");
        scanf("\n%c", &algo_choice);
    } while (algo_choice != '1' && algo_choice != '2');

    // Read through the input file and output each virtual address
    while (fgets(addressReadBuffer, MAX_ADDR_LEN, address_file) != NULL)
    {
        virtual_addr = atoi(addressReadBuffer); // converting from ascii to int

        // 32-bit masking function to extract page number
        page_number = getPageNumber(PAGE_MASK, virtual_addr, SHIFT);

        // 32-bit masking function to extract page offset
        offset_number = getOffset(OFFSET_MASK, virtual_addr);

        // Get the physical address and translatedValue stored at that address
        translateAddress(algo_choice);
        translationCount++; // increment the number of translated addresses
    }

    // Determining stdout algo name for Menu
    if (algo_choice == '1')
    {
        algoName = "FIFO";
    }
    else
    {
        algoName = "LRU";
    }

    printf("\n-----------------------------------------------------------------------------------\n");
    // calculate and print out the stats
    printf("\nResults Using %s Algorithm: \n", algoName);
    printf("Number of translated addresses = %d\n", translationCount);
    double pfRate = (double)pageTable->pageFaultCount / (double)translationCount;
    double TLBRate = (double)tlbTable->tlbHitCount / (double)translationCount;

    printf("Page Faults = %d\n", pageTable->pageFaultCount);
    printf("Page Fault Rate = %.3f %%\n", pfRate * 100);
    printf("TLB Hits = %d\n", tlbTable->tlbHitCount);
    printf("TLB Hit Rate = %.3f %%\n", TLBRate * 100);
    printf("Average time spent retrieving data from backing store: %.3f millisec\n", getAvgTimeInBackingStore());
    printf("\n-----------------------------------------------------------------------------------\n");

    // close the input file and backing store
    fclose(address_file);
    fclose(backing_store);

    // NOTE: REMEMBER TO FREE DYNAMICALLY ALLOCATED MEMORY
    freeVmTable(&tlbTable);
    freeVmTable(&pageTable);
    freeDRAM(&dram, TOTAL_FRAME_COUNT);
    return 0;
}

/*
    This function takes the virtual address and translates
    into physical addressing, and retrieves the translatedValue stored
    @Param algo_type Is '1' for tlbFIFOinsert and '2' for tlbLRUinsert
 */
void translateAddress()
{
    // First try to get page from TLB
    int frame_number = -1; // Assigning -1 to frame_number means invalid initially

    /*
        Look through the TLB to see if memory page exists.
        If found, extract frame number and increment hit count
    */
    for (int i = 0; i < tlbTable->length; i++)
    {
        if (tlbTable->pageNumArr[i] == page_number)
        {
            frame_number = tlbTable->frameNumArr[i];
            tlbTable->tlbHitCount++;
            break;
        }
    }

    /*
        We now need to see if there was a TLB miss.
        If so, increment the tlb miss count and go through
        the page table to see if the page number exists. If not on page table,
        read secondary storage (dram) and increment page table fault count.
    */
    if (frame_number == -1)
    {
        tlbTable->tlbMissCount++; // Increment the miss count
        // walk the contents of the page table
        for (int i = 0; i < nextPage; i++)
        {
            if (pageTable->pageNumArr[i] == page_number)
            {                                             // If the page is found in those contents
                frame_number = pageTable->frameNumArr[i]; // Extract the frame_number
                break;                                    // Found in pageTable
            }
        }
        // NOTE: Page Table Fault Encountered
        if (frame_number == -1)
        {                                // if the page is not found in those contents
            pageTable->pageFaultCount++; // Increment the number of page faults
            // page fault, call to readFromStore to get the frame into physical memory and the page table
            start = clock();
            readFromStore(page_number);
            cpu_time_used += (double)(clock() - start) / CLOCKS_PER_SEC;
            functionCallCount++;
            frame_number = nextFrame - 1; // And set the frame_number to the current nextFrame index
        }
    }

    if (algo_choice == '1')
    {
        // Call function to insert the page number and frame number into the TLB
        tlbFIFOinsert(page_number, frame_number);
    }
    else
    {
        tlbLRUinsert(page_number, frame_number);
    }

    // Frame number and offset used to get the signed translatedValue stored at that address
    translatedValue = dram[frame_number][offset_number];

    // FOR TESTING -- printf("\nFrame Number: %d; Offset: %d; ", frame_number, offset_number);

    if (display_choice == 'y')
    {
        // output the virtual address, physical address and translatedValue of the signed char to the console
        printf("\nVirtual address: %d\t\tPhysical address: %d\t\tValue: %d", virtual_addr, (frame_number << SHIFT) | offset_number, translatedValue);
    }
}

// Function to read from the backing store and bring the frame into physical memory and the page table
void readFromStore(int pageNumber)
{
    // first seek to byte PAGE_READ_SIZE in the backing store
    // SEEK_SET in fseek() seeks from the beginning of the file
    if (fseek(backing_store, pageNumber * PAGE_READ_SIZE, SEEK_SET) != 0)
    {
        fprintf(stderr, "Error seeking in backing store\n");
    }

    // now read PAGE_READ_SIZE bytes from the backing store to the fileReadBuffer
    if (fread(fileReadBuffer, sizeof(signed char), PAGE_READ_SIZE, backing_store) == 0)
    {
        fprintf(stderr, "Error reading from backing store\n");
    }

    // Load the bits into the first available frame in the physical memory 2D array
    for (int i = 0; i < PAGE_READ_SIZE; i++)
    {
        dram[nextFrame][i] = fileReadBuffer[i];
    }

    // Then we want to load the frame number into the page table in the next frame
    pageTable->pageNumArr[nextPage] = pageNumber;
    pageTable->frameNumArr[nextPage] = nextFrame;

    // increment the counters that track the next available frames
    nextFrame++;
    nextPage++;
}

// Function to insert a page number and frame number into the TLB with a FIFO replacement
void tlbFIFOinsert(int pageNumber, int frameNumber)
{
    int i;

    // If it's already in the TLB, break
    for (i = 0; i < nextTLBentry; i++)
    {
        if (tlbTable->pageNumArr[i] == pageNumber)
        {
            break;
        }
    }

    // if the number of entries is equal to the index
    if (i == nextTLBentry)
    {
        if (nextTLBentry < TLB_SIZE)
        {                                                    // IF TLB Buffer has open positions
            tlbTable->pageNumArr[nextTLBentry] = pageNumber; // insert the page and frame on the end
            tlbTable->frameNumArr[nextTLBentry] = frameNumber;
        }
        else
        { // No room in TLB Buffer

            // Replace the last TLB entry with our new entry
            tlbTable->pageNumArr[nextTLBentry - 1] = pageNumber;
            tlbTable->frameNumArr[nextTLBentry - 1] = frameNumber;

            // Then shift up all the TLB entries by 1 so we can maintain FIFO order
            for (i = 0; i < TLB_SIZE - 1; i++)
            {
                tlbTable->pageNumArr[i] = tlbTable->pageNumArr[i + 1];
                tlbTable->frameNumArr[i] = tlbTable->frameNumArr[i + 1];
            }
        }
    }

    // If the index is not equal to the number of entries
    else
    {

        for (i = i; i < nextTLBentry - 1; i++)
        { // iterate through up to one less than the number of entries
            // Move everything over in the arrays
            tlbTable->pageNumArr[i] = tlbTable->pageNumArr[i + 1];
            tlbTable->frameNumArr[i] = tlbTable->frameNumArr[i + 1];
        }
        if (nextTLBentry < TLB_SIZE)
        { // if there is still room in the array, put the page and frame on the end
            // Insert the page and frame on the end
            tlbTable->pageNumArr[nextTLBentry] = pageNumber;
            tlbTable->frameNumArr[nextTLBentry] = frameNumber;
        }
        else
        { // Otherwise put the page and frame on the number of entries - 1
            tlbTable->pageNumArr[nextTLBentry - 1] = pageNumber;
            tlbTable->frameNumArr[nextTLBentry - 1] = frameNumber;
        }
    }
    if (nextTLBentry < TLB_SIZE)
    { // If there is still room in the arrays, increment the number of entries
        nextTLBentry++;
    }
}

// Function to insert a page number and frame number into the TLB with a LRU replacement
void tlbLRUinsert(int pageNumber, int frameNumber)
{

    bool freeSpotFound = false;
    bool alreadyThere = false;
    int replaceIndex = -1;

    // SEEK -- > Find the index to replace and increment age for all other entries
    for (int i = 0; i < TLB_SIZE; i++)
    {
        if ((tlbTable->pageNumArr[i] != pageNumber) && (tlbTable->pageNumArr[i] != 0))
        {
            // If entry is not in TLB and not a free spot, increment its age
            tlbTable->entryAgeArr[i]++;
        }
        else if ((tlbTable->pageNumArr[i] != pageNumber) && (tlbTable->pageNumArr[i] == 0))
        {
            // Available spot in TLB found
            if (!freeSpotFound)
            {
                replaceIndex = i;
                freeSpotFound = true;
            }
        }
        else if (tlbTable->pageNumArr[i] == pageNumber)
        {
            // Entry is already in TLB -- Reset its age
            if (!alreadyThere)
            {
                tlbTable->entryAgeArr[i] = 0;
                alreadyThere = true;
            }
        }
    }

    // REPLACE
    if (alreadyThere)
    {
        return;
    }
    else if (freeSpotFound)
    {                                                    // If we found a free spot, insert
        tlbTable->pageNumArr[replaceIndex] = pageNumber; // Insert into free spot
        tlbTable->frameNumArr[replaceIndex] = frameNumber;
        tlbTable->entryAgeArr[replaceIndex] = 0;
    }
    else
    { // No more free spots -- Need to replace the oldest entry
        replaceIndex = getOldestEntry(TLB_SIZE);
        tlbTable->pageNumArr[replaceIndex] = pageNumber; // Insert into oldest entry
        tlbTable->frameNumArr[replaceIndex] = frameNumber;
        tlbTable->entryAgeArr[replaceIndex] = 0;
    }
}

// Finds the oldest entry in TLB and returns the replacement Index
int getOldestEntry(int tlbSize)
{
    int i, max, index;

    max = tlbTable->entryAgeArr[0];
    index = 0;

    for (i = 1; i < tlbSize; i++)
    {
        if (tlbTable->entryAgeArr[i] > max)
        {
            index = i;
            max = tlbTable->entryAgeArr[i];
        }
    }
    return index;
}

// Just computes the average time complexity of accessing the backing store
double getAvgTimeInBackingStore()
{
    double temp = (double)cpu_time_used / (double)functionCallCount;
    return temp * 1000000;
}

// #define VIRTUAL_MEMORY_SIZE (1 << 14)                            // 16KB
// #define PHYSICAL_MEMORY_SIZE (1 << 13)                           // 8KB
// #define PAGE_SIZE (1 << 2)                                       // 4B
// #define PAGE_MASK 0xFF00                                         // Masks everything but the page number
// #define OFFSET_MASK 0xFF                                         // Masks everything but the offset
// #define MAX_PAGE_TABLE_ENTRIES (VIRTUAL_MEMORY_SIZE / PAGE_SIZE) // 2^18 entries
// #define MAX_FRAME_ENTRIES (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)     // 2^16 entries
// #define MAX_TLB_ENTRIES 8                                        // 16 entries
// #define NUM_SIMULATED_PROCESSES 10                               // 10 processes

// #define ADDRESS_BUFFER_LEN 10 // 10 bytes
// #define DISK_BUFFER_LEN 256    // 256 bytes

// // page table , translation lookaside buffer and main memory repr
// vmTable_t *pageDirectory;
// vmTable_t *tlbCache;
// int **dram;

// // file descriptors to address file (with 14 bit virtual addresses) and disk file (with 13 bit physical addresses)
// FILE *address_file;
// FILE *disk_file;

// // read buffers for address and disk files @TODO: specify buffer size
// char addressReadBuffer[ADDRESS_BUFFER_LEN];
// char diskReadBuffer[DISK_BUFFER_LEN];

// int nextTLBentry = 0; // Tracks the next available index of entry into the TLB
// int nextPage = 0;     // Tracks the next available page in the page table
// int nextFrame = 0;    // Tracks the next available frame TLB or Page Table

// // Generating length of time for a function
// clock_t start, end;
// double cpu_time_used;
// int functionCallCount = 0;

// int virtual_addr;
// int page_number;
// int offset_number;

// signed char translatedValue;

// // hoisting function prototypes
// void translateAddress();
// void tlbInsertFIFO();
// void tlbInsertLRU(int pageNumber, int frameNumber);
// void readFromDisk(int pageNumber);
// int getOldestEntry(int tlbSize);
// double getAvgTimeInBackingStore();

// int main(int argc, char const *argv[])
// {

//     // Initialize the PD, TLB, and main memory
//     tlbCache = createVmTable(MAX_TLB_ENTRIES);
//     pageDirectory = createVmTable(MAX_PAGE_TABLE_ENTRIES); // @TODO: Change to PD (Page Directory)
//     dram = createDRAM(MAX_FRAME_ENTRIES, PAGE_SIZE);

//     // open file with randomly generated addresses
//     address_file = fopen("./data/address_file.txt", "r");
//     if (address_file == NULL)
//     {
//         fprintf(stderr, "Error opening address file");
//         return -1;
//     }

//     disk_file = fopen("./data/disk_file.bin", "rb");
//     if (disk_file == NULL)
//     {
//         fprintf(stderr, "Error opening disk file");
//         return -1;
//     }

//     // read and allocate addresses to X amount of processes
//     while (fgets(addressReadBuffer, ADDRESS_BUFFER_LEN, address_file) != NULL)
//     {
//         int virtual_addr = atoi(addressReadBuffer);
//         // 32-bit masking function to extract page number
//         page_number = getPageNumber(PAGE_MASK, virtual_addr, 8);

//         // 32-bit masking function to extract page offset
//         offset_number = getOffset(OFFSET_MASK, virtual_addr);

//         translateAddress();
//     }

//     printf("\n-----------------------------------------------------------------------------------\n");
//     // calculate and print out the stats
//     // printf("\nResults Using %s Algorithm: \n", algoName);
//     // printf("Number of translated addresses = %d\n", translationCount);
//     double pfRate = (double)pageDirectory->pageFaultCount / (double)1000;
//     double TLBRate = (double)tlbCache->tlbHitCount / (double)1000;

//     printf("Page Faults = %d\n", pageDirectory->pageFaultCount);
//     printf("Page Fault Rate = %.3f %%\n", pfRate * 100);
//     printf("TLB Hits = %d\n", tlbCache->tlbHitCount);
//     printf("TLB Hit Rate = %.3f %%\n", TLBRate * 100);
//     printf("Average time spent retrieving data from backing store: %.3f millisec\n", getAvgTimeInBackingStore());
//     printf("\n-----------------------------------------------------------------------------------\n");

//     // close address file and disk file
//     fclose(address_file);
//     fclose(disk_file);

//     // deallocate dynamically allocated memory for tables and main memory
//     freeVmTable(&tlbCache);
//     freeVmTable(&pageDirectory);
//     freeDRAM(&dram, MAX_FRAME_ENTRIES);

//     return 0;
// }

// void translateAddress()
// {
//     // First try to get page from TLB
//     int frame_number = -1; // Assigning -1 to frame_number means invalid initially

//     /*
//         Look through the TLB to see if memory page exists.
//         If found, extract frame number and increment hit count
//     */
//     for (int i = 0; i < tlbCache->length; i++)
//     {
//         if (tlbCache->pageNumArr[i] == page_number)
//         {
//             frame_number = tlbCache->frameNumArr[i];
//             tlbCache->tlbHitCount++;
//             break;
//         }
//     }

//     /*
//         We now need to see if there was a TLB miss.
//         If so, increment the tlb miss count and go through
//         the page table to see if the page number exists. If not on page table,
//         read secondary storage (dram) and increment page table fault count.
//     */
//     if (frame_number == -1)
//     {
//         tlbCache->tlbMissCount++; // Increment the miss count
//         // walk the contents of the page table
//         for (int i = 0; i < nextPage; i++)
//         {
//             if (pageDirectory->pageNumArr[i] == page_number)
//             {                                                 // If the page is found in those contents
//                 frame_number = pageDirectory->frameNumArr[i]; // Extract the frame_number
//                 break;                                        // Found in pageDirectory
//             }
//         }
//         // NOTE: Page Table Fault Encountered
//         if (frame_number == -1)
//         {                                    // if the page is not found in those contents
//             pageDirectory->pageFaultCount++; // Increment the number of page faults
//             // page fault, call to readFromDisk to get the frame into physical memory and the page table
//             start = clock();
//             readFromDisk(page_number);
//             cpu_time_used += (double)(clock() - start) / CLOCKS_PER_SEC;
//             functionCallCount++;
//             frame_number = nextFrame - 1; // And set the frame_number to the current nextFrame index
//         }
//     }

//     tlbInsertLRU(page_number, frame_number);

//     // Frame number and offset used to get the signed translatedValue stored at that address
//     translatedValue = dram[frame_number][offset_number];

//     // FOR TESTING -- printf("\nFrame Number: %d; Offset: %d; ", frame_number, offset_number);

//     // if (display_choice == 'y')
//     // {
//     // output the virtual address, physical address and translatedValue of the signed char to the console
//     printf("\nVirtual address: %d\t\tPhysical address: %d\t\tValue: %d", virtual_addr, (frame_number << 8) | offset_number, translatedValue);
//     // }
// }

// void tlbInsertFIFO()
// {
// }

// void tlbInsertLRU(int pageNumber, int frameNumber)
// {
//     bool freeSpotFound = false;
//     bool alreadyThere = false;
//     int replaceIndex = -1;

//     // SEEK -- > Find the index to replace and increment age for all other entries
//     for (int i = 0; i < MAX_TLB_ENTRIES; i++)
//     {
//         if ((tlbCache->pageNumArr[i] != pageNumber) && (tlbCache->pageNumArr[i] != 0))
//         {
//             // If entry is not in TLB and not a free spot, increment its age
//             tlbCache->entryAgeArr[i]++;
//         }
//         else if ((tlbCache->pageNumArr[i] != pageNumber) && (tlbCache->pageNumArr[i] == 0))
//         {
//             // Available spot in TLB found
//             if (!freeSpotFound)
//             {
//                 replaceIndex = i;
//                 freeSpotFound = true;
//             }
//         }
//         else if (tlbCache->pageNumArr[i] == pageNumber)
//         {
//             // Entry is already in TLB -- Reset its age
//             if (!alreadyThere)
//             {
//                 tlbCache->entryAgeArr[i] = 0;
//                 alreadyThere = true;
//             }
//         }
//     }

//     // REPLACE
//     if (alreadyThere)
//     {
//         return;
//     }
//     else if (freeSpotFound)
//     {                                                    // If we found a free spot, insert
//         tlbCache->pageNumArr[replaceIndex] = pageNumber; // Insert into free spot
//         tlbCache->frameNumArr[replaceIndex] = frameNumber;
//         tlbCache->entryAgeArr[replaceIndex] = 0;
//     }
//     else
//     { // No more free spots -- Need to replace the oldest entry
//         replaceIndex = getOldestEntry(MAX_TLB_ENTRIES);
//         tlbCache->pageNumArr[replaceIndex] = pageNumber; // Insert into oldest entry
//         tlbCache->frameNumArr[replaceIndex] = frameNumber;
//         tlbCache->entryAgeArr[replaceIndex] = 0;
//     }
// }

// void readFromDisk(int pageNumber)
// {
//     // first seek to byte PAGE_READ_SIZE in the backing store
//     // SEEK_SET in fseek() seeks from the beginning of the file
//     if (fseek(disk_file, pageNumber * DISK_BUFFER_LEN, SEEK_SET) != 0)
//     {
//         fprintf(stderr, "Error seeking in backing store\n");
//     }

//     // now read PAGE_READ_SIZE bytes from the backing store to the fileReadBuffer
//     if (fread(diskReadBuffer, sizeof(signed char), DISK_BUFFER_LEN, disk_file) == 0)
//     {
//         fprintf(stderr, "Error reading from backing store\n");
//     }

//     // Load the bits into the first available frame in the physical memory 2D array
//     for (int i = 0; i < DISK_BUFFER_LEN; i++)
//     {
//         dram[nextFrame][i] = diskReadBuffer[i];
//     }

//     // Then we want to load the frame number into the page table in the next frame
//     pageDirectory->pageNumArr[nextPage] = pageNumber;
//     pageDirectory->frameNumArr[nextPage] = nextFrame;

//     // increment the counters that track the next available frames
//     nextFrame++;
//     nextPage++;
// }

// // Finds the oldest entry in TLB and returns the replacement Index
// int getOldestEntry(int tlbSize)
// {
//     int i, max, index;

//     max = tlbCache->entryAgeArr[0];
//     index = 0;

//     for (i = 1; i < tlbSize; i++)
//     {
//         if (tlbCache->entryAgeArr[i] > max)
//         {
//             index = i;
//             max = tlbCache->entryAgeArr[i];
//         }
//     }
//     return index;
// }

// // Just computes the average time complexity of accessing the backing store
// double getAvgTimeInBackingStore()
// {
//     double temp = (double)cpu_time_used / (double)functionCallCount;
//     return temp * 1000000;
// }
